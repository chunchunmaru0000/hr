#include "pser.h"

// ###########################################################################################
// 											+
// ###########################################################################################
struct GlobExpr *glob_add_two_ints(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->number += r->tvar->number;
	free_glob_expr(r);
	return l;
}
struct GlobExpr *glob_add_two_reals(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real += r->tvar->real;
	free_glob_expr(r);
	return l;
}
struct GlobExpr *glob_add_real_and_int(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real += r->tvar->number;
	free_glob_expr(r);
	return l;
}
// ###########################################################################################
// 											-
// ###########################################################################################
struct GlobExpr *glob_sub_two_ints(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->number -= r->tvar->number;
	free_glob_expr(r);
	return l;
}
struct GlobExpr *glob_sub_two_reals(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real -= r->tvar->real;
	free_glob_expr(r);
	return l;
}
struct GlobExpr *glob_sub_real_and_int(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real -= r->tvar->number;
	free_glob_expr(r);
	return l;
}
struct GlobExpr *glob_sub_int_and_real(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->number -= r->tvar->real;
	free_glob_expr(r);
	return l;
}
// ###########################################################################################
//
// ###########################################################################################
struct GlobExpr *global_addng(struct Pser *p, struct GlobExpr *l,
							  struct GlobExpr *r, struct Token *op) {
	if (op->code == PLUS) {
		if (l->code == CT_INT && r->code == CT_INT)
			l = glob_add_two_ints(l, r);
		else if (l->code == CT_REAL && r->code == CT_REAL)
			l = glob_add_two_reals(l, r);
		else if (l->code == CT_REAL && r->code == CT_INT)
			l = glob_add_real_and_int(l, r);
		else if (l->code == CT_INT && r->code == CT_REAL)
			l = glob_add_real_and_int(r, l);
		else
			exit(220); // just for now later do it not as a черт
	} else {		   // MINUS
		if (l->code == CT_INT && r->code == CT_INT)
			l = glob_sub_two_ints(l, r);
		else if (l->code == CT_REAL && r->code == CT_REAL)
			l = glob_sub_two_reals(l, r);
		else if (l->code == CT_REAL && r->code == CT_INT)
			l = glob_sub_real_and_int(l, r);
		else if (l->code == CT_INT && r->code == CT_REAL)
			l = glob_sub_int_and_real(l, r);
		else
			exit(219);
	}
	return l;
}
