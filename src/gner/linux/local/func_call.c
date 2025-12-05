#include "../../gner.h"
#include <stdio.h>

#define rsi g->cpu->si

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

struct Reg *call_to_reg(Gg, struct LocalExpr *e) {
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
		op_(CALL);
		blat_ft(fun_gvar->signature), ft_add('\n');
	}

	for (i = 0; i < ops_regs->size; i++)
		free_reg_family(((struct Reg *)plist_get(ops_regs, i))->rf);
	plist_free(ops_regs);

	int res_size = unsafe_size_of_type(e->type);
	if (r1 == 0)
		r1 = try_borrow_reg(e->tvar, g, res_size);
	else
		r1 = reg_of_sz(r1->rf, res_size);

	get_reg_to_rf(e->tvar, g, r1, g->cpu->a);
	return r1;
}

void gen_call(Gg, struct LocalExpr *e) {
	gen_tuple_of(g, e);

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
		op_(CALL);
		blat_ft(fun_gvar->signature), ft_add('\n');
	}

	for (i = 0; i < ops_regs->size; i++)
		free_reg_family(((struct Reg *)plist_get(ops_regs, i))->rf);
	plist_free(ops_regs);

	free_reg_rf_if_not_zero(r1);
}
