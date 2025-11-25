#include "../../gner.h"
#include <stdio.h>

struct Reg *index_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	;
	return 0;
}

struct Reg *inc_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	;
	return 0;
}

struct Reg *dec_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	;
	return 0;
}

// for field_of_ptr_to_reg and field_to_reg
u8 during_add = 0;

struct Reg *field_of_ptr_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1;
	struct Arg *arg = (struct Arg *)e->tvar->num;
	struct BList *field_full_name = (struct BList *)(long)e->tvar->real;

	during_add == 0 ? (reg_size = arg->arg_size) : (during_add = 0);

	// printf("### field_of_ptr_to_reg [%s] during_add = %d of size %d\n",
	// 	   bs(field_full_name), during_add, reg_size);

	r1 = gen_to_reg(g, e->l, QWORD);
	op_reg_(MOV, r1->reg_code);
	sib_enter(reg_size, 0, 0, r1->reg_code, field_full_name, 1);

	blist_clear_free(field_full_name);
	return r1;
}
struct Reg *field_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1;
	struct Arg *arg = (struct Arg *)e->tvar->num;
	struct BList *field_full_name = (struct BList *)(long)e->tvar->real;
	enum RegCode base;

	if (during_add == 0)
		reg_size = arg->arg_size;

	// printf("### field_to_reg [%s] during_add = %d of size %d\n",
	// 	   bs(field_full_name), during_add, reg_size);

	if (lceep(e->l, VAR)) {
		declare_lvar_gvar;
		get_assignee_size(g, e->l, &gvar, &lvar);

		blist_add(field_full_name, '+');
		lvar ? (blat_blist(field_full_name, lvar->name->view), base = R_RBP)
			 : (blat_blist(field_full_name, gvar->signature), base = 0);

		r1 = try_borrow_reg(e->tvar, g, reg_size);
		op_reg_(MOV, r1->reg_code);
		sib_enter(reg_size, 0, 0, base, field_full_name, 1);

		if (during_add)
			g->fun_text->size -= 2; // remove ")\n" after sib_enter

	} else if (lceeu(e->l, ADDR)) {
		if (during_add) {
			during_add = 0;

			r1 = gen_to_reg(g, e->l->l, QWORD);
			op_reg_(MOV, r1->reg_code);
			sib_enter(reg_size, 0, 0, r1->reg_code, field_full_name, 1);

			g->fun_text->size -= 2; // remove ")\n" after sib_enter
		} else {
			r1 = gen_to_reg(g, e->l->l, QWORD);
			op_reg_(MOV, r1->reg_code);
			sib_enter(reg_size, 0, 0, r1->reg_code, field_full_name, 1);
		}

	} else if (lceea(e->l, FIELD_OF_PTR)) {
		if (during_add) {
			r1 = field_of_ptr_to_reg(g, e->l, reg_size);

			g->fun_text->size -= 2; // remove ")\n" after sib_enter
			ft_add('+');
			blat_ft(field_full_name);
		} else {
			during_add = 1;
			r1 = field_of_ptr_to_reg(g, e->l, reg_size);

			g->fun_text->size -= 2; // remove ")\n" after sib_enter
			ft_add('+');
			blat_ft(field_full_name);
			ft_add(')'), ft_add('\n');
		}
	} else if (lceea(e->l, FIELD)) {
		if (during_add) {
			r1 = field_to_reg(g, e->l, reg_size);
			ft_add('+');
			blat_ft(field_full_name);
		} else {
			during_add = 1;
			r1 = field_to_reg(g, e->l, reg_size);

			ft_add('+');
			blat_ft(field_full_name);
			ft_add(')'), ft_add('\n');
		}
	} else {
		printf("[%d]\n", e->l->code);
		eet(e->tvar, "эээ че type 0, где define_type", 0);
	}

	during_add = 0;
	blist_clear_free(field_full_name);
	return r1;
}

// returns PList of Reg's
struct PList *gen_ops_to_regs(Gg, struct LocalExpr *e, struct PList *ops) {
	u32 i;
	struct TypeExpr *arg_type;
}

struct Reg *call_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct LocalExpr *fun_expr = e->l;
	struct GlobVar *fun_gvar;
	struct PList *ops_regs;
	u32 i;
	struct Reg *r1 = 0, *tmp_r;

	exit(147);
	ops_regs = gen_ops_to_regs(g, e, e->co.ops);
	// save changable regs before call
	save_allocated_regs(g, e->tvar);

	if (e->tvar->num == 0) {
		r1 = gen_to_reg(g, fun_expr, QWORD);
		op_reg_enter(CALL, r1->reg_code);
	} else {
		fun_gvar = (struct GlobVar *)e->tvar->num;
		isprint_ft(CALL);
		blat_ft(fun_gvar->signature), ft_add('\n');
	}

	for (i = 0; i < ops_regs->size; i++) {
		tmp_r = plist_get(ops_regs, i);
		free_reg_family(tmp_r->rf);
	}
	if (r1 == 0)
		r1 = try_borrow_reg(e->tvar, g, unsafe_size_of_type(e->type));

	get_reg_to_rf(e->tvar, g, r1, g->cpu->a);
	return r1;
}

struct Reg *after_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1 = 0;

	if (lcea(INDEX)) {
	} else if (lcea(CALL))
		r1 = call_to_reg(g, e, reg_size);
	else if (lcea(INC)) {
	} else if (lcea(DEC)) {
	} else if (lcea(FIELD_OF_PTR))
		r1 = field_of_ptr_to_reg(g, e, reg_size);
	else if (lcea(FIELD))
		r1 = field_to_reg(g, e, reg_size);

	if (!r1)
		exit(146);
	return r1;
}
