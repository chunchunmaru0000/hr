#include "../../gner.h"
#include <stdio.h>

#define rsi g->cpu->si

// returns PList of Reg's
struct PList *mov_ops_regs_to_args_regs(struct Token *place, Gg,
										struct PList *fun_args_types,
										struct PList *ops) {
	struct RegisterFamily **cpu_regs;
	struct LocalExpr *argument;
	int fun_arg_size;
	struct Reg *r;
	u32 i;
	//  save changable regs before call
	struct PList *ops_regs = new_plist(ops->size);
	save_allocated_regs(g, place);

	for (i = 0, cpu_regs = &rsi; i < ops->size; i++, cpu_regs++) {
		argument = plist_get(ops, i);
		fun_arg_size = unsafe_size_of_type(plist_get(fun_args_types, i));
		// gen reg
		if (lceep(argument, REAL)) {
			gen_tuple_of(g, argument);
			r = try_borrow_reg(
				place, g, argument->type->code == TC_SINGLE ? DWORD : QWORD);
			if (argument->tvar->real) {
				op_reg_(MOV, r->reg_code);
				real_add_enter(fun_text, argument->tvar->real);
			} else {
				op_reg_reg(XOR, r, r);
			}
		} else
			r = gen_to_reg(g, argument, fun_arg_size);
		// change reg to basic if its xmm
		if (is_xmm(r))
			r = cvt_from_xmm(g, plist_get(ops, i), r);
		else
			r = get_reg_to_size(g, r, fun_arg_size);

		plist_add(ops_regs, r);
	}
	for (i = 0, cpu_regs = &rsi; i < ops->size; i++, cpu_regs++) {
		r = plist_get(ops_regs, i);
		get_reg_to_rf(place, g, r, *cpu_regs);
		plist_set(ops_regs, i, (*cpu_regs)->r);
	}

	return ops_regs;
}

void gen_call(Gg, struct LocalExpr *e) {
	struct LocalExpr *fun_expr = e->l;
	struct PList *ops_regs;

	g->flags->is_stack_used = 1;

	if (e->r == 0) {
		struct Reg *r = gen_to_reg(g, fun_expr, QWORD);

		ops_regs = mov_ops_regs_to_args_regs(
			e->tvar, g, fun_args(fun_expr->type), e->co.ops);

		op_reg_enter(CALL, r->reg_code);
		free_reg_family(r->rf);
	} else {
		struct GlobVar *fun_gvar = (void *)e->r;
		gen_tuple_of(g, fun_expr);

		ops_regs = mov_ops_regs_to_args_regs(
			e->tvar, g, fun_args(fun_gvar->type), e->co.ops);

		op_(CALL);
		blat_ft(fun_gvar->signature), ft_add('\n');
	}

	for (u32 i = 0; i < ops_regs->size; i++)
		free_reg_family(((struct Reg *)plist_get(ops_regs, i))->rf);
	plist_free(ops_regs);
}

struct Reg *call_to_reg(Gg, struct LocalExpr *e) {
	gen_call(g, e);

	struct Reg *r = try_borrow_reg(e->tvar, g, unsafe_size_of_type(e->type));
	get_reg_to_rf(e->tvar, g, r, g->cpu->a);
	return r;
}
