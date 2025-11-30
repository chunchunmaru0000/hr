#include "../../gner.h"

constr NOT_ASSIGNABLE = "Данное выражение не может быть присваеваемым.";

void assign_to_mem(Gg, struct LocalExpr *e) {
	struct LocalExpr *mem = e->l;
	struct LocalExpr *assignable = e->r;
	struct Reg *reg;

	gen_mem_tuple(g, mem);

	if (lceep(assignable, INT)) {
		gen_tuple_of(g, assignable);
		op_mem_(MOV, mem, 0);
		add_int_with_hex_comm(fun_text, assignable->tvar->num);

	} else if (lceep(assignable, REAL)) {
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
	struct Reg *r1 = 0, *r2 = 0;
	struct BList *last_mem_str = 0;

	r1 = gen_to_reg_with_last_mem(g, assignee, trailed, &last_mem_str);

	if (is_num_le(right)) {

		if (lceep(right, INT)) {
			isprint_ft(MOV);
			blat_blist(g->fun_text, last_mem_str);
			add_int_with_hex_comm(fun_text, right->tvar->num);
		} else {
			if (is_ss(right->type)) {
				isprint_ft(MOV);
				blat_blist(g->fun_text, last_mem_str);
				real_add_enter(fun_text, right->tvar->real);
			} else {
				r2 = try_borrow_reg(right->tvar, g, QWORD);
				op_reg_(MOV, r2->reg_code);
				real_add_enter(fun_text, right->tvar->real);

				isprint_ft(MOV);
				blat_blist(g->fun_text, last_mem_str);
				reg_enter(r2->reg_code);
			}
		}
	} else {
		r2 = gen_to_reg(g, right, 0);

		isprint_ft(MOV);
		blat_blist(g->fun_text, last_mem_str);
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
