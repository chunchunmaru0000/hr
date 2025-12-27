#include "../../gner.h"
#include <stdio.h>

#define rsi g->cpu->si
#define xmm9 g->cpu->xmm[9]

// returns PList of Reg's
struct PList *mov_ops_regs_to_args_regs(struct Token *place, Gg,
										struct PList *fun_args_types,
										struct PList *ops) {
	struct RegisterFamily **rfs_regs = &rsi;
	struct Reg **xmm_regs = &xmm9;

	struct LocalExpr *argument;
	struct TypeExpr *arg_type;
	struct RegisterFamily *rf;
	struct Reg *r, *to_xmm;
	u32 i, regov = 0, xmmov = 0;

	//  save changable regs before call
	struct PList *ops_regs = new_plist(ops->size);
	save_allocated_regs(g, place);

	for (i = 0; i < ops->size; i++) {
		argument = plist_get(ops, i);
		arg_type = plist_get(fun_args_types, i);

		if (is_real_type(arg_type)) {
			to_xmm = *(xmm_regs++);
			r = return_to_xmm(g, arg_type, argument, to_xmm);
		} else {
			rf = *(rfs_regs++);
			r = return_to_rf(g, arg_type, argument, rf);
		}
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

	struct Reg *r;

	if (is_real_type(e->type)) {
		r = try_borrow_xmm_reg(e->tvar, g);
		get_xmm_to_xmm(g, r, g->cpu->xmm[0]);
	} else {
		r = try_borrow_reg(e->tvar, g, unsafe_size_of_type(e->type));
		get_reg_to_rf(e->tvar, g, r, g->cpu->a);
	}
	return r;
}
