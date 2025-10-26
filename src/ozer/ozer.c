#include "ozer.h"

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

// 1 + a + 1
void opt_bin_constant_folding(struct LocalExpr *e) {
	struct LocalExpr *l, *r, *cond;

	if (is_bin_le(e) || e->code == LE_BIN_ASSIGN) {
		l = e->l, r = e->r;
		opt_bin_constant_folding(l);
		opt_bin_constant_folding(r);

		if (is_num_le(l) && is_num_le(r)) {
			if (e->code == LE_BIN_PLUS) {
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
		}

	} else if (e->code == LE_BIN_TERRY) {
		cond = e->co.cond, l = e->l, r = e->r;

		opt_bin_constant_folding(cond);
		opt_bin_constant_folding(l);
		opt_bin_constant_folding(r);
	} else {
	}
}
#include <stdio.h>
struct PList *opt_local_expr(struct LocalExpr *e) {
	struct PList *es = new_plist(1);

	opt_bin_constant_folding(e);
	plist_add(es, e);

	return es;
}
