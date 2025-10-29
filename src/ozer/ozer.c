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

e * 0 -> 0
e / 0 -> ERROR
e + - 0 -> e
e * / 1 -> e
x e + x e -> 2 x e, то есть множители
делители и типа все другое
*/

void bin_l_and_r_to_e(struct LocalExpr *l, struct LocalExpr *r,
					  struct LocalExpr *e, enum LE_Code op_code) {
	if (op_code != LE_BIN_PLUS)
		return;

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

	// - TODO: free in this fiel elsewhere
	// local_expr_free(l);
	// local_expr_free(r);
}

#define fnia(root_place)                                                       \
	(find_num_in_adds((root_place), &found_num, &found_num_bin_bro,            \
					  &found_num_parrent_place, op_code))

// TODO: need to rewrite as loop for speed and mem
void find_num_in_adds(struct LocalExpr **root_place_in_parrent,
					  struct LocalExpr **found_num,
					  struct LocalExpr **found_num_bin_bro,
					  struct LocalExpr ***found_num_parrent_place,
					  enum LE_Code op_code) {
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
	if (root->l->code == op_code) {
		find_num_in_adds(&root->l, found_num, found_num_bin_bro,
						 found_num_parrent_place, op_code);
		if (*found_num)
			return;
	}
	if (root->r->code == op_code) {
		find_num_in_adds(&root->r, found_num, found_num_bin_bro,
						 found_num_parrent_place, op_code);
		if (*found_num)
			return;
	}

	*found_num = 0;
	*found_num_bin_bro = 0;
	*found_num_parrent_place = 0;
}

void try_bin_num_in_bin(struct LocalExpr *num,
						struct LocalExpr **root_place_in_parrent,
						enum LE_Code op_code) {
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
		if ((*root_place_in_parrent)->code != op_code)
			break;
		fnia(root_place_in_parrent);

		if (!found_num)
			break;

		bin_l_and_r_to_e(num, found_num, num, op_code);
		*found_num_parrent_place = found_num_bin_bro;

		// free(found_num);
		// free(found_num_parrent_place); ?
	}
}

void try_bin_bins(struct LocalExpr *e) {
	struct LocalExpr *found_num;
	struct LocalExpr *found_num_bin_bro;
	struct LocalExpr **found_num_parrent_place;
	enum LE_Code op_code = e->code;

	if (e->l->code == op_code) {
		fnia(&e->l);
		if (found_num) {
			try_bin_num_in_bin(found_num, &e->r, op_code);
			return;
		}
	}
	if (e->r->code == op_code) {
		fnia(&e->r);
		if (found_num) {
			try_bin_num_in_bin(found_num, &e->l, op_code);
			return;
		}
	}
	// free(found_num);
	// free(found_num_parrent_place); ?
}

// 1 + a + 1
void opt_bin_constant_folding(struct LocalExpr *e) {
	struct LocalExpr *l, *r, *cond;

	if (is_bin_le(e) || e->code == LE_BIN_ASSIGN) {
		l = e->l, r = e->r;
		opt_bin_constant_folding(l);
		opt_bin_constant_folding(r);

		if (is_bin_le(e)) {
			if (is_num_le(l) && is_num_le(r)) {
				bin_l_and_r_to_e(l, r, e, e->code);
			} else if (is_num_le(l) && is_bin_le(r)) {
				try_bin_num_in_bin(l, &e->r, e->code);
			} else if (is_bin_le(l) && is_num_le(r)) {
				try_bin_num_in_bin(r, &e->l, e->code);
			} else if (is_bin_le(l) && is_bin_le(r)) {
				try_bin_bins(e);
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
