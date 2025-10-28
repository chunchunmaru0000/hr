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
		// free_old_view
		e->tvar->view = real_to_str(e->tvar->real);
	} else {
		e->code = LE_PRIMARY_INT;
		e->tvar->num = l->tvar->num + r->tvar->num;
		// free_old_view
		e->tvar->view = int_to_str(e->tvar->num);
	}
	zero_term_blist(e->tvar->view);

	// - TODO:
	// local_expr_free(l);
	// local_expr_free(r);
}

void find_num_in_adds(struct LocalExpr **root_place_in_parrent,
					  struct LocalExpr **found_num,
					  struct LocalExpr **found_num_bin_bro,
					  struct LocalExpr ***found_num_parrent_place) {
	struct LocalExpr *root = *root_place_in_parrent;

	if (is_num_le(root->l)) {
		*found_num = root->l;
		*found_num_bin_bro = root->r;
		*found_num_parrent_place = root_place_in_parrent;
		return;
	}
	if (is_num_le(root->r)) {
		*found_num = root->r;
		*found_num_bin_bro = root->l;
		*found_num_parrent_place = root_place_in_parrent;
		return;
	}
	if (is_add_le(root->l)) {
		find_num_in_adds(&root->l, found_num, found_num_bin_bro,
						 found_num_parrent_place);
		if (*found_num)
			return;
	}
	if (is_add_le(root->r)) {
		find_num_in_adds(&root->r, found_num, found_num_bin_bro,
						 found_num_parrent_place);
		if (*found_num)
			return;
	}

	*found_num = 0;
	*found_num_bin_bro = 0;
	*found_num_parrent_place = 0;
}

void try_add_num_in_bin(struct LocalExpr *num,
						struct LocalExpr **root_place_in_parrent) {
	struct LocalExpr *found_num;
	struct LocalExpr *found_num_bin_bro;
	struct LocalExpr **found_num_parrent_place;

	// l I have alredy here
	// found_num == r->r
	// add it to l
	// found_num_bin_bro == r->l
	// found_num_parrent_place == e->r
	// found_num_bin_bro becomes found_num_parrent_place
	loop {
		if (!is_add_le(*root_place_in_parrent))
			break;
		find_num_in_adds(root_place_in_parrent, &found_num, &found_num_bin_bro,
						 &found_num_parrent_place);
		if (!found_num)
			break;

		add_l_and_r_to_e(num, found_num, num);
		*found_num_parrent_place = found_num_bin_bro;

		// free(found_num);
		// free(found_num_parrent_place); ?
	}
}

// 1 + a + 1
void opt_bin_constant_folding(struct LocalExpr *e) {
	struct LocalExpr *l, *r, *cond, *num;
	struct Token *op;

	if (is_bin_le(e) || e->code == LE_BIN_ASSIGN) {
		l = e->l, r = e->r;
		opt_bin_constant_folding(l);
		opt_bin_constant_folding(r);

		if (is_add_le(e)) {
			if (is_num_le(l) && is_num_le(r)) {
				add_l_and_r_to_e(l, r, e);
			} else if (is_num_le(l) && is_bin_le(r)) {
				try_add_num_in_bin(l, &e->r);
			} else if (is_bin_le(l) && is_num_le(r)) {
				try_add_num_in_bin(r, &e->l);
			} else if (is_bin_le(l) && is_bin_le(r)) {
// 				op = new_tok(copy_blist_from_str("+"), PLUS, e->tvar->p);
// 				num = new_local_expr(LE_PRIMARY_INT, 0, op);
// 
// 				try_add_num_in_bin(num, &e->l);
// 				try_add_num_in_bin(num, &e->r);
// 
// 				if ((is_INT_le(num) && num->tvar->num != 0) ||
// 					(is_REAL_le(num) && num->tvar->real != 0)) {
// 					num = local_bin(e, num, op);
// 					paste_le(e, num);
// 					// same as e = local_bin(num, e, op);
// 				} else {
// 					full_free_token_without_pos(op);
// 					free(num);
// 				}
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
