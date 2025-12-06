#include "../../gner.h"

struct Reg *terry_to_reg(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0, *r2 = 0;
	int res_size = unsafe_size_of_type(e->type);

	struct BList *r_result_label, *exit_label;

	r_result_label = take_label(g, LC_ELSE);
	exit_label = take_label(g, LC_ELSE);

	and_cmp(g, e->co.cond, r_result_label);

	// l
	r1 = gen_to_reg(g, e->l, res_size);
	free_reg_family(r1->rf);
	// jmp exit
	jmp_(exit_label);
	// r
	add_label(r_result_label);
	r2 = gen_to_reg(g, e->r, res_size);
	get_reg_to_rf(e->tvar, g, r2, r1->rf);
	// exit_label:
	add_label(exit_label);

	blist_clear_free(r_result_label);
	blist_clear_free(exit_label);

	if (!r2)
		exit(193);
	return r2;
}

void gen_terry(Gg, struct LocalExpr *e) {
	struct BList *r_result_label = take_label(g, LC_ELSE), *exit_label;

	if (causes_more_than_just_gvar(e->l) && causes_more_than_just_gvar(e->r)) {
		exit_label = take_label(g, LC_ELSE);
		and_cmp(g, e->co.cond, r_result_label);

		// l
		gen_local_expr_linux(g, e->l);
		jmp_(exit_label);
		// r
		add_label(r_result_label);
		gen_local_expr_linux(g, e->r);
		// exit_label:
		add_label(exit_label);
		blist_clear_free(exit_label);

	} else if (causes_more_than_just_gvar(e->l)) {
		and_cmp(g, e->co.cond, r_result_label);
		// l
		gen_local_expr_linux(g, e->l);
		// r
		add_label(r_result_label);

	} else if (causes_more_than_just_gvar(e->r)) {
		or_cmp(g, e->co.cond, r_result_label);
		// r
		gen_local_expr_linux(g, e->r);
		add_label(r_result_label);

	} else
		exit(190);

	blist_clear_free(r_result_label);
}

void gen_if_else(Gg, struct LocalExpr *e) {
	struct PList *os;
	u32 i;
	struct BList *r_label = take_label(g, LC_ELSE),
				 *exit_label = take_label(g, LC_ELSE);

	and_cmp(g, e->co.cond, r_label);
	g->indent_level++;
	// l
	for (os = (void *)e->l, i = 0; i < os->size; i++)
		gen_local_linux(g, plist_get(os, i));
	plist_free(os);
	jmp_(exit_label);
	// r
	add_label(r_label);
	for (os = (void *)e->r, i = 0; i < os->size; i++)
		gen_local_linux(g, plist_get(os, i));
	plist_free(os);
	// exit_label:
	g->indent_level--;
	add_label(exit_label);

	blist_clear_free(exit_label);
	blist_clear_free(r_label);
}
