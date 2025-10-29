#include "ozer.h"

constr LE_DIV_ON_ZERO = "ЭЭЭ ты куда на 0 делишь.";

// e * 1 -> e
// e * 0 -> 0
int try_opt_mul(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0)) {
		paste_le(e, e->l);
		opted = 1;
	} else if (is_le_num(e->r, 0)) {
		paste_le(e, e->r);
		opted = 1;
	} else if (is_le_num(e->l, 1)) {
		paste_le(e, e->r);
		opted = 1;
	} else if (is_le_num(e->r, 1)) {
		paste_le(e, e->l);
		opted = 1;
	}
	return opted;
}

// e / 1 -> e
// e / 0 -> ERR
int try_opt_div(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0)) {
		eet(e->l->tvar, LE_DIV_ON_ZERO, 0);
	} else if (is_le_num(e->r, 0)) {
		eet(e->l->tvar, LE_DIV_ON_ZERO, 0);
	} else if (is_le_num(e->l, 1)) {
		paste_le(e, e->r);
		opted = 1;
	} else if (is_le_num(e->r, 1)) {
		paste_le(e, e->l);
		opted = 1;
	}
	return opted;
}

// e +, - 0 -> e
// e <<, >> 0 -> e
int try_opt_add_or_sub(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0)) {
		paste_le(e, e->r);
		opted = 1;
	} else if (is_le_num(e->r, 0)) {
		paste_le(e, e->l);
		opted = 1;
	}
	return opted;
}

// e && false -> false
int try_opt_and(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0)) { // 0 is false
		paste_le(e, e->l);
		opted = 1;
	} else if (is_le_num(e->r, 0)) { // 0 is false
		paste_le(e, e->r);
		opted = 1;
	}
	return opted;
}

// e || true -> true
int try_opt_or(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_not_num(e->l, 0)) {
		paste_le(e, e->l);
		opted = 1;
	} else if (is_le_not_num(e->r, 0)) {
		paste_le(e, e->r);
		opted = 1;
	}
	if (opted) {
		e->tvar->num = 1;
		e->tvar->real = 1;
	}
	return opted;
}

// e | 0 -> e
int try_opt_bit_or(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0)) {
		paste_le(e, e->r);
		opted = 1;
	} else if (is_le_num(e->r, 0)) {
		paste_le(e, e->l);
		opted = 1;
	}
	return opted;
}
