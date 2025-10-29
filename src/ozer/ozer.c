#include "ozer.h"
#include <stdio.h>

#define cant_on_reals(op_name, op)                                             \
	constr CANT_##op_name##_ON_REALS =                                         \
		"Операция '" op "' не применима к вещественным числам.";

cant_on_reals(MOD, "%");
cant_on_reals(SHL, "<<");
cant_on_reals(SHR, ">>");
cant_on_reals(BIT_AND, "&");
cant_on_reals(BIT_XOR, "^");
cant_on_reals(BIT_OR, "|");

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
e && false -> 0
e || true -> 1
e * / 1 -> e
x e + x e -> 2 x e, то есть множители
делители и типа все другое
*/

#define real_op(op) (e->tvar->real = l->tvar->real op r->tvar->real)
#define int_op(op) (e->tvar->num = l->tvar->num op r->tvar->num)

void bin_l_and_r_to_e(struct LocalExpr *l, struct LocalExpr *r,
					  struct LocalExpr *e, enum LE_Code op_code) {
	if (is_INT_le(l))
		l->tvar->real = l->tvar->num;
	if (is_INT_le(r))
		r->tvar->real = r->tvar->num;

	if (is_REAL_le(l) || is_REAL_le(r)) {

		if (op_code == LE_BIN_MUL)
			real_op(*);
		else if (op_code == LE_BIN_DIV)
			real_op(/);
		else if (op_code == LE_BIN_MOD)
			eet(e->tvar, CANT_MOD_ON_REALS, 0);
		else if (op_code == LE_BIN_WHOLE_DIV) {
			real_op(/);
			e->tvar->num = e->tvar->real;
			goto do_int;
		} else if (op_code == LE_BIN_PLUS)
			real_op(+);
		else if (op_code == LE_BIN_MINUS)
			real_op(-);
		else if (op_code == LE_BIN_SHL)
			eet(e->tvar, CANT_SHL_ON_REALS, 0);
		else if (op_code == LE_BIN_SHR)
			eet(e->tvar, CANT_SHR_ON_REALS, 0);
		else if (op_code == LE_BIN_LESS)
			real_op(<);
		else if (op_code == LE_BIN_LESSE)
			real_op(<=);
		else if (op_code == LE_BIN_MORE)
			real_op(>);
		else if (op_code == LE_BIN_MOREE)
			real_op(>=);
		else if (op_code == LE_BIN_EQUALS)
			real_op(==);
		else if (op_code == LE_BIN_NOT_EQUALS)
			real_op(!=);
		else if (op_code == LE_BIN_BIT_AND)
			eet(e->tvar, CANT_BIT_AND_ON_REALS, 0);
		else if (op_code == LE_BIN_BIT_XOR)
			eet(e->tvar, CANT_BIT_XOR_ON_REALS, 0);
		else if (op_code == LE_BIN_BIT_OR)
			eet(e->tvar, CANT_BIT_OR_ON_REALS, 0);
		else if (op_code == LE_BIN_AND)
			real_op(&&);
		else if (op_code == LE_BIN_OR)
			real_op(||);
		else
			eet(e->tvar, "real эээ", 0);

		e->code = LE_PRIMARY_REAL;
		blist_clear_free(e->tvar->view);
		e->tvar->view = real_to_str(e->tvar->real);
	} else {

		if (op_code == LE_BIN_MUL)
			int_op(*);
		else if (op_code == LE_BIN_DIV)
			int_op(/);
		else if (op_code == LE_BIN_MOD)
			eet(e->tvar, "TODO: сделать mod для целых", 0);
		else if (op_code == LE_BIN_WHOLE_DIV)
			int_op(/);
		else if (op_code == LE_BIN_PLUS)
			int_op(+);
		else if (op_code == LE_BIN_MINUS)
			int_op(-);
		else if (op_code == LE_BIN_SHL)
			int_op(<<);
		else if (op_code == LE_BIN_SHR)
			int_op(>>);
		else if (op_code == LE_BIN_LESS)
			int_op(<);
		else if (op_code == LE_BIN_LESSE)
			int_op(<=);
		else if (op_code == LE_BIN_MORE)
			int_op(>);
		else if (op_code == LE_BIN_MOREE)
			int_op(>=);
		else if (op_code == LE_BIN_EQUALS)
			int_op(==);
		else if (op_code == LE_BIN_NOT_EQUALS)
			int_op(!=);
		else if (op_code == LE_BIN_BIT_AND)
			int_op(&);
		else if (op_code == LE_BIN_BIT_XOR)
			int_op(^);
		else if (op_code == LE_BIN_BIT_OR)
			int_op(|);
		else if (op_code == LE_BIN_AND)
			int_op(&&);
		else if (op_code == LE_BIN_OR)
			int_op(||);
		else
			eet(e->tvar, "int эээ", 0);

	do_int:
		e->code = LE_PRIMARY_INT;
		blist_clear_free(e->tvar->view);
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
