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
		real_add(g->fun_text, assignable->tvar->real);
		ft_add('\n');
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

// struct LocalExpr *find_trailed(struct LocalExpr *e)
// 	return lcea(FIELD) || index_of_int ? find_trailed(e->l) : e;
// struct LocalExpr *is_not_assignable_or_trailed(struct LocalExpr *e)
// 	return lcea(FIELD) || index_of_int		  ? find_trailed(e->l)
// 		   : lcea(FIELD_OF_PTR) || lceu(ADDR) ? e
// 											  : 0;
void assign_to_last_mem(Gg, struct LocalExpr *assignee,
						struct LocalExpr *trailed, struct LocalExpr *right) {
	struct Reg *r1, *r2;
	struct BList *last_mem_str = 0;

	r1 = gen_to_reg_with_last_mem(g, assignee, trailed, &last_mem_str);
}

void gen_assign(struct Gner *g, struct LocalExpr *e) {
	struct LocalExpr *assignee = e->l, *trailed;

	if (is_mem(assignee))
		assign_to_mem(g, e);
	else if ((trailed = is_not_assignable_or_trailed(e)))
		assign_to_last_mem(g, assignee, trailed, e->r);
	else
		eet(e->l->tvar, NOT_ASSIGNABLE, 0);
}
