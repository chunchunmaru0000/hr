#include "../../gner.h"
#include <stdio.h>

// for field_of_ptr_to_reg and field_to_reg
u8 during_add = 0;

struct Reg *index_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1, *r2 = 0;
	struct LocalExpr *left = e->l, *trailed;
	struct TypeExpr *iant_type = left->type;
	int item_size;

	if (during_add) {
		reg_size = QWORD;
		during_add = 0;
	}
	item_size = iant_type->code == TC_PTR
					? unsafe_size_of_type(ptr_targ(iant_type))
					: unsafe_size_of_type(arr_type(iant_type));
	if (lceep(e->r, INT)) {
		long unit = item_size * e->r->tvar->num;

		if (iant_type->code == TC_PTR) {
			r1 = gen_to_reg(g, left, QWORD);
			op_reg_(MOV, r1->reg_code);
			sib_enter(QWORD, r1->reg_code, 0, 0, unit, 0);

		} else if (iant_type->code == TC_ARR) {
			if ((trailed = is_not_assignable_or_trailed(left))) {
				struct BList *last_mem_str = new_blist(64);
				r1 = last_inner_mem(g, left, trailed, last_mem_str);

				op_reg_(MOV, r1->reg_code);
				blat_ft(size_str(reg_size));
				ft_add('(');
				blat_ft(last_mem_str);
				if (unit) {
					sprint_ft(MEM_PLUS);
					int_add(g->fun_text, unit);
				}
				ft_add(')'), ft_add('\n');

				blist_clear_free(last_mem_str);
			} else
				exit(180);
		}
	} else {
		r2 = gen_to_reg(g, e->r, QWORD); // index

		if (iant_type->code == TC_PTR) {
			r1 = gen_to_reg(g, left, QWORD);

			if (is_in_word(item_size)) {
				op_reg_(MOV, r1->reg_code);
				sib_enter(QWORD, r1->reg_code, item_size, r2->reg_code, 0, 0);
			} else {
				mul_on_int(g, r2, item_size);
				op_reg_(MOV, r1->reg_code);
				sib_enter(QWORD, r1->reg_code, 0, r2->reg_code, 0, 0);
			}
		} else if (iant_type->code == TC_ARR) {
			if (is_mem(left)) {
				if (is_in_word(item_size)) {
					op_reg_(MOV, r2->reg_code);
					blat_ft(size_str(reg_size));
					ft_add('(');
					int_add(g->fun_text, item_size);
					ft_add(' ');
				} else {
					mul_on_int(g, r2, item_size);

					op_reg_(MOV, r2->reg_code);
					blat_ft(size_str(reg_size));
					ft_add('(');
				}
				reg_(r2->rf->r->reg_code);
				inner_mem(g, e);
				ft_add(')'), ft_add('\n');

				r1 = r2, r2 = 0;

			} else if ((trailed = is_not_assignable_or_trailed(left))) {
				struct BList *last_mem_str = new_blist(64);
				r1 = last_inner_mem(g, left, trailed, last_mem_str);

				isprint_ft(MOV);
				blat_ft(last_mem_str);
				ft_add(' '), reg_enter(r2->reg_code);

				blist_clear_free(last_mem_str);
			} else
				exit(176);
		}
	}

	free_reg_rf_if_not_zero(r2);
	return r1;
}

struct Reg *field_of_ptr_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1;
	struct Arg *arg = (struct Arg *)e->tvar->num;
	struct BList *field_full_name = (struct BList *)(long)e->tvar->real;

	during_add == 0 ? (reg_size = arg->arg_size) : (during_add = 0);

	r1 = gen_to_reg(g, e->l, QWORD);
	op_reg_(MOV, r1->reg_code);
	sib_enter(reg_size, 0, 0, r1->reg_code, field_full_name, 1);

	blist_clear_free(field_full_name);
	return r1;
}

struct Reg *field_to_reg_during_add(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1;
	struct BList *field_full_name = (struct BList *)(long)e->tvar->real;
	struct LocalExpr *left = e->l;

	if (is_mem(left)) {
		gen_mem_tuple(g, left);
		mem_(g, e->l, reg_size);
		g->fun_text->size--; // remove " " after mem_

	} else if (lceeu(left, ADDR)) {
		r1 = gen_to_reg(g, e, QWORD);
		g->fun_text->size -= 2; // remove ")\n" after sib_enter
		blat_ft(field_full_name);

	} else if (lceea(left, FIELD_OF_PTR)) {
		r1 = field_of_ptr_to_reg(g, left, reg_size);
		g->fun_text->size -= 2; // remove ")\n" after sib_enter
		ft_add('+'), blat_ft(field_full_name);

	} else if (lceea(left, FIELD)) {
		r1 = field_to_reg_during_add(g, left, reg_size);
		ft_add('+'), blat_ft(field_full_name);

	} else if (lceea(left, INDEX)) {
		r1 = index_to_reg(g, left, reg_size);
		g->fun_text->size -= 2; // remove ")\n" after sib_enter
		ft_add('+'), blat_ft(field_full_name);

	} else {
		printf("[%d]\n", left->code);
		eet(e->tvar, "╤Н╤Н╤Н ╤З╨╡ type 0, ╨│╨┤╨╡ define_type", 0);
	}

	during_add = 0;
	blist_clear_free(field_full_name);
	return r1;
}

struct Reg *field_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1;
	struct Arg *arg = (struct Arg *)e->tvar->num;
	struct BList *field_full_name = (struct BList *)(long)e->tvar->real;
	struct LocalExpr *left = e->l;

	reg_size = arg->arg_size;

	if (is_mem(left)) {
		gen_mem_tuple(g, left);
		mem_enter(e->l, reg_size);

	} else if (lceeu(left, ADDR)) {
		r1 = gen_to_reg(g, left, QWORD);
		op_reg_(MOV, r1->reg_code);
		sib_enter(reg_size, 0, 0, r1->reg_code, field_full_name, 1);

	} else if (lceea(left, FIELD_OF_PTR)) {
		during_add = 1;
		r1 = field_of_ptr_to_reg(g, left, reg_size);
		g->fun_text->size -= 2; // remove ")\n" after sib_enter
		ft_add('+'), blat_ft(field_full_name);
		ft_add(')'), ft_add('\n');

	} else if (lceea(left, FIELD)) {
		during_add = 1;
		r1 = field_to_reg_during_add(g, left, reg_size);
		ft_add('+'), blat_ft(field_full_name);
		ft_add(')'), ft_add('\n');

	} else if (lceea(left, INDEX)) {
		during_add = 1;
		r1 = index_to_reg(g, left, reg_size);
		g->fun_text->size -= 2; // remove ")\n" after sib_enter
		ft_add('+'), blat_ft(field_full_name);
		ft_add(')'), ft_add('\n');

	} else {
		printf("[%d]\n", left->code);
		eet(e->tvar, "эээ че type 0, где define_type", 0);
	}

	during_add = 0;
	blist_clear_free(field_full_name);
	return r1;
}

#define rsi g->cpu->si
#define rdi g->cpu->di
#define r8 g->cpu->rex[8 - 8]
#define r9 g->cpu->rex[9 - 8]
#define r10 g->cpu->rex[10 - 8]
#define r11 g->cpu->rex[11 - 8]
#define r12 g->cpu->rex[12 - 8]

// returns PList of Reg's
struct PList *mov_ops_regs_to_args_regs(struct Token *place, Gg,
										struct PList *ops) {
	struct RegisterFamily **cpu_regs;
	struct LocalExpr *argument;
	struct Reg *r;
	u32 i;
	//  save changable regs before call
	struct PList *ops_regs = new_plist(ops->size);
	save_allocated_regs(g, place);

	for (i = 0, cpu_regs = &rsi; i < ops->size; i++, cpu_regs++) {
		argument = plist_get(ops, i);
		// gen reg
		if (lceep(argument, REAL)) {
			r = try_borrow_reg(
				place, g, argument->type->code == TC_SINGLE ? DWORD : QWORD);
			if (argument->tvar->real) {
				op_reg_(MOV, r->reg_code);
				real_add_enter(fun_text, argument->tvar->real);
			} else {
				op_reg_reg(XOR, r, r);
			}
		} else
			r = gen_to_reg(g, argument, 0);
		// change reg to basic if its xmm
		if (is_xmm(r))
			r = cvt_from_xmm(g, plist_get(ops, i), r);

		plist_add(ops_regs, r);
	}
	for (i = 0, cpu_regs = &rsi; i < ops->size; i++, cpu_regs++) {
		r = plist_get(ops_regs, i);
		get_reg_to_rf(place, g, r, *cpu_regs);
		plist_set(ops_regs, i, (*cpu_regs)->r);
	}

	return ops_regs;
}

struct Reg *call_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct LocalExpr *fun_expr = e->l;
	struct GlobVar *fun_gvar;
	struct PList *ops_regs;
	u32 i;
	struct Reg *r1 = 0;

	g->flags->is_stack_used = 1;

	if (e->tvar->num == 0) {
		r1 = gen_to_reg(g, fun_expr, QWORD);
		ops_regs = mov_ops_regs_to_args_regs(e->tvar, g, e->co.ops);

		op_reg_enter(CALL, r1->reg_code);
	} else {
		gen_tuple_of(g, fun_expr);
		ops_regs = mov_ops_regs_to_args_regs(e->tvar, g, e->co.ops);

		fun_gvar = (struct GlobVar *)e->tvar->num;
		isprint_ft(CALL);
		blat_ft(fun_gvar->signature), ft_add('\n');
	}

	for (i = 0; i < ops_regs->size; i++)
		free_reg_family(((struct Reg *)plist_get(ops_regs, i))->rf);
	plist_free(ops_regs);

	if (r1 == 0)
		r1 = try_borrow_reg(e->tvar, g, unsafe_size_of_type(e->type));

	get_reg_to_rf(e->tvar, g, r1, g->cpu->a);
	return r1;
}

struct Reg *after_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1 = 0;

	if (lcea(INDEX)) {
		r1 = index_to_reg(g, e, reg_size);
	} else if (lcea(CALL))
		r1 = call_to_reg(g, e, reg_size);
	else if (lcea(INC) || lcea(DEC)) {
		r1 = after_dec_inc(g, e->l, lcea(INC));
	} else if (lcea(FIELD_OF_PTR))
		r1 = field_of_ptr_to_reg(g, e, reg_size);
	else if (lcea(FIELD))
		r1 = field_to_reg(g, e, reg_size);

	if (!r1)
		exit(146);
	return r1;
}
