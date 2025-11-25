#include "gner.h"

struct Gner *new_gner(struct Pser *p, enum Target tget, uc debug) {
	struct Gner *g = malloc(sizeof(struct Gner));

	g->t = tget;
	g->debug = debug;

	g->is = pse(p);
	g->cpu = new_cpu();
	g->indent_level = 0;
	g->pos = 0;
	g->stack_counter = 0;

	g->enums = p->enums;
	g->flags = malloc(sizeof(struct Fggs));

	g->labels = malloc(sizeof(struct Lbls));
	g->labels->loops = 0;
	g->labels->whiles = 0;
	g->labels->fors = 0;
	g->labels->ifs = 0;
	g->labels->elses = 0;
	g->labels->ptrs = 0;

	g->bprol = new_blist(128);
	g->prol = new_blist(128);
	g->aprol = new_blist(128);
	g->text = new_blist(128);
	g->stack_frame = new_blist(128);
	g->after_stack_frame = new_blist(128);
	g->fun_text = new_blist(128);

	g->global_vars = p->global_vars;
	g->same_name_funs = p->same_name_funs;
	g->local_vars = new_plist(16);
	g->local_labels = new_plist(8);

	g->tmp_blist = 0;

	// struct PList *ts; // tokens
	plist_free(p->local_vars);
	pser_err(p);
	free(p);

	return g;
}

void reset_flags(struct Gner *g) {
	struct Fggs *f = g->flags;
	f->is_stack_used = 0;
	f->is_r13_used = 0;
	f->is_r14_used = 0;
	f->is_r15_used = 0;
	f->need_save_args_on_stack_count = 0;
	f->is_args_in_regs = 1;
	f->is_data_segment_used = 0;
}

void gen(struct Gner *g) {
	switch (g->t) {
	case T_Асм_Linux_64:
		gen_linux_text(g);
		break;
	}
}

struct LocalVar *new_local_var(struct Token *name, struct Arg *arg,
							   long stack_pointer) {
	struct LocalVar *var = malloc(sizeof(struct LocalVar));
	var->name = name;
	var->stack_pointer = stack_pointer;
	var->type = arg->type;
	var->lvar_size = arg->arg_size;
	return var;
}

void free_and_clear_local_vars(struct Gner *g) {
	struct LocalVar *var;
	uint32_t i;
	void *last_freed = 0;

	for (i = 0; i < g->local_vars->size; i++) {
		var = plist_get(g->local_vars, i);

		if (var->type == last_freed)
			goto skip_freed_pointer_free;

		free_type(var->type);
		last_freed = var->type;

	skip_freed_pointer_free:;
		free(var);
	}

	plist_clear(g->local_vars);
}

void write_fun(struct Gner *g) {
	blat_text(g->stack_frame);
	blat_text(g->after_stack_frame);
	blat_text(g->fun_text);

	blist_clear(g->stack_frame);
	blist_clear(g->after_stack_frame);
	blist_clear(g->fun_text);
}

void indent_line(struct Gner *g, struct BList *l) {
	for (uint32_t i = g->indent_level; i; i--)
		blist_add(l, '\t');
}

constr TOO_COMPLEX_EXPR = "Не хватило регистров для вычисления выражения.";
constr MAKE_SIMPLER_EXPR = "разделить выражение на несколько";

#define rc(r, reg) ((r)->reg_code == R_##reg)
#define is_r13(r)                                                              \
	(rc((r), R13) || rc((r), R13D) || rc((r), R13W) || rc((r), R13B))
#define is_r14(r)                                                              \
	(rc((r), R14) || rc((r), R14D) || rc((r), R14W) || rc((r), R14B))
#define is_r15(r)                                                              \
	(rc((r), R15) || rc((r), R15D) || rc((r), R15W) || rc((r), R15B))

constr TOO_BIG_EXPR_FOR_REG =
	"Слишком большое или маленькое выражение для помещения его в регистр.";
constr SIZE_IN_BYTES = "размер выражения в байтах: ";

void err_of_size(struct Token *place, long size) {
	struct ErrorInfo info = {
		place, TOO_BIG_EXPR_FOR_REG, SIZE_IN_BYTES, (void *)size, ET_INT,
	};
	etei_with_extra(&info);
	exit(1);
}

struct Reg *try_borrow_reg(struct Token *place, Gg, uc of_size) {
	if (of_size > QWORD || of_size < BYTE)
		err_of_size(place, of_size);

	struct Reg *reg = borrow_basic_reg(g->cpu, of_size);

	if (reg == 0)
		eet(place, TOO_COMPLEX_EXPR, MAKE_SIMPLER_EXPR);
	if (is_r13(reg))
		g->flags->is_r13_used = 1;
	else if (is_r14(reg))
		g->flags->is_r14_used = 1;
	else if (is_r15(reg))
		g->flags->is_r15_used = 1;

	return reg;
}

struct Reg *try_borrow_xmm_reg(struct Token *place, Gg) {
	struct Reg *reg = borrow_xmm_reg(g->cpu);
	if (reg == 0)
		eet(place, TOO_COMPLEX_EXPR, MAKE_SIMPLER_EXPR);
	return reg;
}

struct Reg *try_alloc_reg(struct Token *tvar, struct RegisterFamily *rf,
						  int size) {
	struct Reg *reg = alloc_reg_of_size(rf, size);
	if (!reg)
		eet(tvar, "wrong reg alloc", 0);
	rf->r->allocated = 1;
	rf->e->allocated = 1;
	rf->x->allocated = 1;
	if (size > BYTE) {
		if (rf->h)
			rf->h->allocated = 1;
		if (rf->l)
			rf->l->allocated = 1;
	} else {
		if (reg == rf->h)
			rf->h->allocated = 1;
		else
			rf->l->allocated = 1;
	}
	return reg;
}

void get_reg_to_rf(struct Token *tvar, Gg, struct Reg *reg,
				   struct RegisterFamily *rf) {
	if (reg->rf != rf) {
		if (rf->r->allocated) {
			// reg is now points to rf's reg
			swap_basic_regs(g, rf, reg->rf, DO_XCHG);
		} else {
			op_reg_reg(MOV, rf->r, reg->rf->r);
			free_reg_family(reg->rf);
			reg = try_alloc_reg(tvar, rf, reg->size);
		}
	}
}

void save_allocated_regs(Gg, struct Token *place) {
	// TODO:
}
