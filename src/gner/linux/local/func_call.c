#include "../../gner.h"
#include <stdio.h>

#define rsi g->cpu->si

// returns PList of Reg's
struct PList *mov_ops_regs_to_args_regs(struct Token *place, Gg,
										struct PList *fun_args_types,
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
		r = try_return_to(g, plist_get(fun_args_types, i), argument, *cpu_regs);
		plist_add(ops_regs, r);
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

		op_reg_enter(CALL, r);
		free_register(r);
	} else {
		struct GlobVar *fun_gvar = (void *)e->r;
		gen_tuple_of(g, fun_expr);

		ops_regs = mov_ops_regs_to_args_regs(
			e->tvar, g, fun_args(fun_gvar->type), e->co.ops);

		op_(CALL);
		blat_ft(fun_gvar->signature), ft_add('\n');
	}

	for (u32 i = 0; i < ops_regs->size; i++)
		free_register(plist_get(ops_regs, i));
	plist_free(ops_regs);
}

struct Reg *call_to_reg(Gg, struct LocalExpr *e) {
	if (e->type->code == TC_TUPLE)
		exit(53);
	gen_call(g, e);

	struct Reg *r = try_borrow_reg(e->tvar, g, unsafe_size_of_type(e->type));
	get_reg_to_rf(e->tvar, g, r, g->cpu->a);
	return r;
}
