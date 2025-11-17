#include "../../gner.h"

void assign_to_var(Gg, struct LocalExpr *e) {
	struct LocalExpr *var = e->l;
	struct LocalExpr *assignable = e->r;
	struct Reg *reg;

	gen_tuple_of(g, var);
	declare_lvar_gvar;
	get_assignee_size(g, var, &gvar, &lvar);

	if (lceep(assignable, INT)) {
		gen_tuple_of(g, assignable);
		mov_var_(g, lvar, gvar);
		add_int_with_hex_comm(fun_text, assignable->tvar->num);

	} else if (lceep(assignable, REAL)) {
		gen_tuple_of(g, assignable);
		mov_var_(g, lvar, gvar);
		real_add(g->fun_text, assignable->tvar->real);
		ft_add('\n');
	} else {
		reg = gen_to_reg(g, assignable, 0);

		if (is_xmm(reg)) {
			mov_xmm_var_(g, lvar, gvar);
			reg_enter(reg->reg_code);
			free_reg(reg);
		} else {
			mov_var_(g, lvar, gvar);
			reg_enter(reg->reg_code);
			free_reg_family(reg->rf);
		}
	}
}

void gen_assign(struct Gner *g, struct LocalExpr *e) {
	struct LocalExpr *assignee = e->l;

	if (lceep(assignee, VAR)) {
		assign_to_var(g, e);
	} else
		exit(161);

	// 	declare_lvar_gvar;
	// 	struct TypeExpr *assignee_type = 0;
	// 	uc assignee_size = 0;
	//
	// 	if (lceeb(assignee, ASSIGN)) {
	// 	} else if (lceep(assignee, VAR)) {
	// 		// assignee_size =
	// 		get_assignee_size(g, assignee, &gvar, &lvar);
	// 		assignee_type = lvar_gvar_type();
	//
	// 		compare_type_and_expr(assignee_type, assignable);
	//
	// 		if (lceep(assignable, INT) || lceep(assignable, REAL)) {
	// 			mov_var_(g, lvar, gvar);
	//
	// 			if (assignable->code == LE_PRIMARY_INT) {
	// 				add_int_with_hex_comm(fun_text, assignable->tvar->num);
	// 			} else {
	// 				real_add(g->fun_text, assignable->tvar->real); // ╤З╨╕╤Б╨╗╨╛
	// 				fun_text_add('\n');							   // \n
	// 			}
	// 		} else {
	// 		}
	// 	}
}
