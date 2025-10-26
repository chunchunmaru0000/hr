#include "ozer.h"
#include <stdio.h>

/*
MUL, DIV [ MOD, WHOLE_DIV ]
PLUS, MINUS
[ SHL, SHR ]
[ LESS, LESSE, MORE, MOREE ]
[ EQUE, NEQU ]
[ AMPER ]
[ BIT_XOR ]
[ BIT_OR ]
[ AND ]
[ OR ]

[ TERRY ]
[ EQU ]
[ PIPE_LINE ]
*/

void add_l_and_r_to_e(struct LocalExpr *l, struct LocalExpr *r,
					  struct LocalExpr *e) {
	if (is_INT_le(l))
		l->tvar->real = l->tvar->num;
	if (is_INT_le(r))
		r->tvar->real = r->tvar->num;

	if (is_REAL_le(l) || is_REAL_le(r)) {
		e->code = LE_PRIMARY_REAL;
		e->tvar->real = l->tvar->real + r->tvar->real;

		e->tvar->view = real_to_str(e->tvar->real);
	} else {
		e->code = LE_PRIMARY_INT;
		e->tvar->num = l->tvar->num + r->tvar->num;

		e->tvar->view = int_to_str(e->tvar->num);
	}
	zero_term_blist(e->tvar->view);

	// - TODO:
	// local_expr_free(l);
	// local_expr_free(r);
}

// 1 + a + 1
void opt_bin_constant_folding(struct LocalExpr *e) {
	struct LocalExpr *l, *r, *cond;

	if (is_bin_le(e) || e->code == LE_BIN_ASSIGN) {
		l = e->l, r = e->r;
		opt_bin_constant_folding(l);
		opt_bin_constant_folding(r);

		if (is_num_le(l) && is_num_le(r)) {
			if (e->code == LE_BIN_PLUS) {
				add_l_and_r_to_e(l, r, e);
			}
		} else if (is_num_le(l) && is_bin_le(r)) {
			if (e->code == LE_BIN_PLUS) {
				if (r->code == LE_BIN_PLUS) {
					if (is_num_le(r->l)) {
						add_l_and_r_to_e(l, r->l, l);
						e->r = r->r;
						// free(r->l);
						// free(r);
					} else if (is_num_le(r->r)) {
						add_l_and_r_to_e(l, r->r, l);
						e->r = r->l;
						// free(r->r);
						// free(r);
					}
				}
			}
		} else if (is_bin_le(l) && is_num_le(r)) {
			if (e->code == LE_BIN_PLUS) {
				if (l->code == LE_BIN_PLUS) {
					if (is_num_le(l->l)) {
						add_l_and_r_to_e(r, l->l, r);
						e->l = l->r;
						// free(l->l);
						// free(l);
					} else if (is_num_le(l->r)) {
						add_l_and_r_to_e(r, l->r, r);
						e->l = l->l;
						// free(l->r);
						// free(l);
					}
				}
			}
		}
	} else if (e->code == LE_BIN_TERRY) {
		cond = e->co.cond, l = e->l, r = e->r;

		opt_bin_constant_folding(cond);
		opt_bin_constant_folding(l);
		opt_bin_constant_folding(r);
	} else {
	}
}

struct PList *opt_local_expr(struct LocalExpr *e) {
	struct PList *es = new_plist(1);

	opt_bin_constant_folding(e);
	plist_add(es, e);

	return es;
}
