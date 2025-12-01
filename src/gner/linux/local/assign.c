#include "../../gner.h"
#include <stdio.h>

constr NOT_ASSIGNABLE = "Данное выражение не может быть присваеваемым.";

#define is_mov_mem_gvar_signature(r)                                           \
	(lceeu((r), AMPER) && lceep((r)->l, VAR) &&                                \
	 (gvar = find_glob_Var(g, (r)->l->tvar->view)))

void assign_to_mem(Gg, struct LocalExpr *e) {
	struct LocalExpr *mem = e->l;
	struct LocalExpr *assignable = e->r;
	struct GlobVar *gvar;
	struct Reg *reg;

	gen_mem_tuple(g, mem);

	if (is_mov_mem_gvar_signature(assignable)) {
		op_mem_(MOV, mem, 0);
		blat_ft(gvar->signature), ft_add('\n');

	} else if (lceep(assignable, INT)) {
		gen_tuple_of(g, assignable);
		op_mem_(MOV, mem, 0);
		add_int_with_hex_comm(fun_text, assignable->tvar->num);

	} else if (lceep(assignable, REAL)) {
		// TODO: mov r, sd; mov mem, r
		gen_tuple_of(g, assignable);
		op_mem_(MOV, mem, 0);
		real_add_enter(fun_text, assignable->tvar->real);

	} else {
		reg = gen_to_reg(g, assignable, 0);

		if (is_xmm(reg)) {
			op_mem_(MOV_XMM, mem, 0);
			reg_enter(reg->reg_code);
			free_reg(reg);
		} else {
			op_mem_(MOV, mem, 0);
			reg_enter(reg->reg_code);
			free_reg_family(reg->rf);
		}
	}
}

void assign_to_last_mem(Gg, struct LocalExpr *assignee,
						struct LocalExpr *trailed, struct LocalExpr *right) {
	struct BList *last_mem_str = 0;
	struct Reg *r1 = 0, *r2 = 0;
	struct GlobVar *gvar;

	r1 = gen_to_reg_with_last_mem(g, assignee, trailed, &last_mem_str);

	if (is_mov_mem_gvar_signature(right)) {
		op_last_mem_(MOV, last_mem_str);
		blat_ft(gvar->signature), ft_add('\n');

	} else if (is_num_le(right)) {
		if (lceep(right, INT)) {
			op_last_mem_(MOV, last_mem_str);
			add_int_with_hex_comm(fun_text, right->tvar->num);
		} else if (is_ss(right->type)) {
			op_last_mem_(MOV, last_mem_str);
			real_add_enter(fun_text, right->tvar->real);
		} else {
			r2 = try_borrow_reg(right->tvar, g, QWORD);
			op_reg_(MOV, r2->reg_code);
			real_add_enter(fun_text, right->tvar->real);

			op_last_mem_(MOV, last_mem_str);
			reg_enter(r2->reg_code);
		}
	} else {
		r2 = gen_to_reg(g, right, 0);
		op_last_mem_(MOV, last_mem_str);
		reg_enter(r2->reg_code);
	}

	free_reg_rf_if_not_zero(r1);
	free_reg_rf_if_not_zero(r2);
	blist_clear_free(last_mem_str);
}

void gen_assign(struct Gner *g, struct LocalExpr *e) {
	struct LocalExpr *assignee = e->l, *trailed;

	if (is_mem(assignee))
		assign_to_mem(g, e);
	else if ((trailed = is_not_assignable_or_trailed(assignee)))
		assign_to_last_mem(g, assignee, trailed, e->r);
	else
		eet(e->l->tvar, NOT_ASSIGNABLE, 0);
}
