#include "../gner/gner.h"

constr LE_DIV_ON_ZERO = "ЭЭЭ ты куда на 0 делишь.";

#define do_opt(thing)                                                          \
	do {                                                                       \
		thing;                                                                 \
		opted = 1;                                                             \
	} while (0)

// e * 1 -> e
// e * 0 -> 0
int try_opt_mul(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0))
		do_opt(paste_le(e, e->l));
	else if (is_le_num(e->r, 0))
		do_opt(paste_le(e, e->r));
	else if (is_le_num(e->l, 1))
		do_opt(paste_le(e, e->r));
	else if (is_le_num(e->r, 1))
		do_opt(paste_le(e, e->l));
	return opted;
}

// e / 1 -> e
// e / 0 -> ERR
int try_opt_div(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0))
		eet(e->l->tvar, LE_DIV_ON_ZERO, 0);
	else if (is_le_num(e->r, 0))
		eet(e->l->tvar, LE_DIV_ON_ZERO, 0);
	else if (is_le_num(e->l, 1))
		do_opt(paste_le(e, e->r));
	else if (is_le_num(e->r, 1))
		do_opt(paste_le(e, e->l));
	return opted;
}

// e +, - 0 -> e
int try_opt_add_or_sub(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0))
		do_opt(paste_le(e, e->r));
	else if (is_le_num(e->r, 0))
		do_opt(paste_le(e, e->l));
	return opted;
}

// e <<, >> 0 -> e
int try_opt_shl_or_shr(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0))
		do_opt(paste_le(e, e->r));
	else if (is_le_num(e->r, 0))
		do_opt(paste_le(e, e->l));
	else if (is_num_le(e->l))
		;
	else if (is_num_le(e->r))
		;
	return opted;
}

// e && false -> false
int try_opt_and(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0)) // 0 is false
		do_opt(paste_le(e, e->l));
	else if (is_le_num(e->r, 0)) // 0 is false
		do_opt(paste_le(e, e->r));
	return opted;
}

// e || true -> true
// e || false -> bool(e), not works for now, cuz how to do bool()
int try_opt_or(struct LocalExpr *e) {
	int opted = 0;
	// if (is_le_num(e->l, 0))
	// 	do_opt(paste_le(e, e->r));
	// else if (is_le_num(e->r, 0))
	// 	do_opt(paste_le(e, e->l));
	// if (opted)
	// 	return 1;
	if (is_le_not_num(e->l, 0))
		do_opt(paste_le(e, e->l));
	else if (is_le_not_num(e->r, 0))
		do_opt(paste_le(e, e->r));

	if (opted) {
		e->tvar->num = 1;
		e->tvar->real = 1;
	}
	return opted;
}

// e | 0 -> e
int try_opt_bit_or(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0))
		do_opt(paste_le(e, e->r));
	else if (is_le_num(e->r, 0))
		do_opt(paste_le(e, e->l));
	return opted;
}

// e & -1 -> e, cuz -1 is definitely 0xFF..FF
// e & 0 -> 0
int try_opt_bit_and(struct LocalExpr *e) {
	int opted = 0;
	if (is_le_num(e->l, 0))
		do_opt(paste_le(e, e->l));
	else if (is_le_num(e->r, 0))
		do_opt(paste_le(e, e->r));
	else if (is_le_num(e->l, -1))
		do_opt(paste_le(e, e->r));
	else if (is_le_num(e->r, -1))
		do_opt(paste_le(e, e->l));
	return opted;
}
