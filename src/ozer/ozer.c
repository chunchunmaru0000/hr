#include "../gner/gner.h"
#include <stdio.h>

/*
TODO:
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

side effects are:
 * dec, int
 * fun call
 * global vars, can ba affected in another thread
e == e -> true,  but if e is not side effective
e != e -> false, but if e is not side effective
e < e  -> false, but if e is not side effective
e > e  -> false, but if e is not side effective
e <= e -> true,  but if e is not side effective
e >= e -> true,  but if e is not side effective

x e + x e -> 2 x e, то есть множители делители и типа все другое
*/
int lee(struct LocalExpr *l, struct LocalExpr *r) {
	if (l->code != r->code)
		return 0;

	u64 i;
	struct LocalExpr *e = l;

	if (is_bin_le(e) || lce(BIN_ASSIGN || lcea(INDEX))) {
		return lee(l->l, r->l) && lee(l->r, r->r);

	} else if (lce(BIN_TERRY)) {
		return lee(l->l, r->l) && lee(l->r, r->r) &&
			   lee(l->co.cond, r->co.cond);

	} else if (lce(PRIMARY_INT)) {
		return l->tvar->num == r->tvar->num;
	} else if (lce(PRIMARY_REAL)) {
		return l->tvar->real == r->tvar->real;
	} else if (lce(PRIMARY_VAR) || lce(PRIMARY_STR)) {
		return vc(l->tvar, r->tvar);

	} else if (lce(PRIMARY_ARR) || lce(PRIMARY_TUPLE)) {
	cmp_ops:
		if (l->co.ops->size != r->co.ops->size)
			return 0;
		for (i = 0; i < l->co.ops->size; i++)
			if (!lee(plist_get(l->co.ops, i), plist_get(r->co.ops, i)))
				return 0;
		return 1;

	} else if (is_unary(e) || lce(BOOL) || lce(AFTER_INC) || lce(AFTER_DEC)) {
		return lee(l->l, r->l);

	} else if (lce(AFTER_CALL)) {
		if (!lee(l->l, r->l))
			return 0;
		goto cmp_ops;

	} else if (lce(AFTER_FIELD_OF_PTR) || lce(AFTER_FIELD)) {
		if (!lee(l->l, r->l))
			return 0;
		return vc(((struct Token *)l->r), ((struct Token *)r->r));
	}

	return 1;
}

void opt_bin_constant_folding(struct LocalExpr *e) {
	struct LocalExpr *l, *r, *cond;

	if (is_bin_le(e)) {
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

		if (if_opted(MUL, mul) || if_opted(DIV, div) ||
			if_opted2(ADD, SUB, add_or_sub) || if_opted(BIT_AND, bit_and) ||
			if_opted2(SHL, SHR, shl_or_shr) || if_opted(AND, and) ||
			if_opted(OR, or) || if_opted(BIT_OR, bit_or) ||
			if_opted2(MORE, LESS, more_or_less) ||
			if_opted2(EQUALS, NOT_EQUALS, eque_or_nequ) ||
			if_opted2(MOREE, LESSE, moree_or_lesse))
			opt_bin_constant_folding(e);

	} else if (lce(BIN_TERRY)) {
		cond = e->co.cond, l = e->l, r = e->r;

		opt_bin_constant_folding(cond);
		opt_bin_constant_folding(l);
		opt_bin_constant_folding(r);
	} else if (lce(BIN_ASSIGN) || lce(AFTER_INDEX)) {
		opt_bin_constant_folding(e->l);
		opt_bin_constant_folding(e->r);
	} else if (is_unary(e) || lce(BOOL)) {
		opt_bin_constant_folding(e->l);
		if (is_num_le(e->l))
			unary_or_bool_of_num(e);
		else
			; // opt_unary_tree(e);
	}
}

struct PList *opt_local_expr(struct LocalExpr *e) {
	struct PList *es = new_plist(1);
	define_type_and_copy_flags_to_e(e);
	opt_bin_constant_folding(e);
	plist_add(es, e);

	return es;
}
