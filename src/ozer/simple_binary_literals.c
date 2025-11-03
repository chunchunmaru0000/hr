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
// e <<, >> n -> e *,/ 2^n , but need to prove that e is int
int try_opt_shl_or_shr(struct LocalExpr *e) {
	int opted = 0;
	if (is_INT_le(e->l)) {
		if (e->l->tvar->num == 0)
			do_opt(paste_le(e, e->r));

	} else if (is_INT_le(e->r)) {
		if (e->r->tvar->num == 0)
			do_opt(paste_le(e, e->l));

		else if (e->l->type && is_int_type(e->l->type)) {
			blist_clear_free(e->tvar->view);

			if (lceb(SHL)) {
				e->code = LE_BIN_MUL;
				e->tvar->view = copy_blist_from_str("*");
			} else {
				e->code = LE_BIN_DIV;
				e->tvar->view = copy_blist_from_str("/");
			}
			e->r->tvar->num = 2 << (e->r->tvar->num - 1);
			update_int_view(e->r);
		}
	}
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
	struct LocalExpr *was_e;
	int opted = 0;

	if (is_le_num(e->l, 0))
		do_opt(paste_le(e, e->r)); // e = e->r
	else if (is_le_num(e->r, 0))
		do_opt(paste_le(e, e->l)); // e = e->l
	if (opted) {				   // e -> bool(e)
		was_e = new_local_expr(LE_NONE, 0, 0);
		paste_le(was_e, e);

		e->code = LE_BOOL;
		e->type = new_type_expr(TC_I32);
		e->l = was_e;
		e->r = 0;
		e->co.ops = 0;
		return 1;
	}

	if (is_le_not_num(e->l, 0))
		do_opt(paste_le(e, e->l));
	else if (is_le_not_num(e->r, 0))
		do_opt(paste_le(e, e->r));
	if (opted) { // e -> true, true is 1, where e is already num
		e->code = LE_PRIMARY_INT;
		e->type->code = TC_I32;
		e->tvar->num = 1;
		update_int_view(e);
		return 1;
	}
	return 0;
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
// e & 0xFFFFFFFF -> e, if e is i32 or u32
// e & 0xFFFF -> e, if e is i16 or u32
// e & 0xFF -> e, if e is i8 or u8
int try_opt_bit_and(struct LocalExpr *e) {
	int opted = 0;
	if (is_num_le(e->l)) {
		if (is_le_num(e->l, 0))
			do_opt(paste_le(e, e->l));
		else if (is_le_num(e->l, -1))
			do_opt(paste_le(e, e->r));
		else if (e->l->type) {
			if ((is_u_or_i_32(e->l->type) && is_le_num(e->l, 0xFFFFFFFF)) ||
				(is_u_or_i_16(e->l->type) && is_le_num(e->l, 0xFFFF)) ||
				(is_u_or_i_8(e->l->type) && is_le_num(e->l, 0xFF)))
				do_opt(paste_le(e, e->r));
		}
	} else if (is_num_le(e->r)) {
		if (is_le_num(e->r, 0))
			do_opt(paste_le(e, e->r));
		else if (is_le_num(e->r, -1))
			do_opt(paste_le(e, e->l));
		else if (e->r->type) {
			if ((is_u_or_i_32(e->r->type) && is_le_num(e->r, 0xFFFFFFFF)) ||
				(is_u_or_i_16(e->r->type) && is_le_num(e->r, 0xFFFF)) ||
				(is_u_or_i_8(e->r->type) && is_le_num(e->r, 0xFF)))
				do_opt(paste_le(e, e->l));
		}
	}
	return opted;
}
