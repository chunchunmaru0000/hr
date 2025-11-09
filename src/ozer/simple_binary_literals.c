#include "../gner/gner.h"
// TODO: mem leaks in this file

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
	if (is_le_num(e->l, 0) && have_only_gvar_effect_or_none(e->r))
		do_opt(paste_le(e, e->l));
	else if (is_le_num(e->r, 0) && have_only_gvar_effect_or_none(e->l))
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

void turn_type_to_i32(struct LocalExpr *e) {
	if (is_simple_type(e->type))
		e->type->code = TC_I32;
	else {
		free_type(e->type);
		e->type = new_type_expr(TC_I32);
	}
}

#define returns_bool(e)                                                        \
	(lce(BOOL) || lceb(AND) || lceb(OR) || lceb(MORE) || lceb(LESS) ||         \
	 lceb(MOREE) || lceb(LESSE) || lceb(EQUALS) || lceb(NOT_EQUALS))

void turn_to_bool(struct LocalExpr *e) {
	if (returns_bool(e))
		return;
	if (e->type)
		free_type(e->type);
	// also free e children

	struct LocalExpr *was_e = new_local_expr(LE_NONE, 0, 0);
	paste_le(was_e, e);

	e->code = LE_BOOL;
	e->type = new_type_expr(TC_I32);
	e->l = was_e;
	e->r = 0;
	e->co.ops = 0;
}

// any part of || or && expression will be already evaluated as bool eventually
int check_if_bools(struct LocalExpr *e) {
	struct LocalExpr *was_e = 0;
	if (lceb(AND) || lceb(OR)) {
		if (e->l->code == LE_BOOL) {
			was_e = e->l->l;
			free(e->l);
			e->l = was_e;
		}
		if (e->r->code == LE_BOOL) {
			was_e = e->r->l;
			free(e->r);
			e->r = was_e;
		}
	}
	return was_e ? 1 : 0;
}

// e && false -> false
// e && true  -> bool(e)
int try_opt_and(struct LocalExpr *e) {
	int opted = 0;

	if (is_le_num(e->l, 0)) // not check sf cuz lazy eval
		do_opt(paste_le(e, e->l));
	else if (is_le_num(e->r, 0) && have_only_gvar_effect_or_none(e->l))
		do_opt(paste_le(e, e->r));
	if (opted) { // e -> false
		turn_type_to_i32(e);
		if (e->code != LE_PRIMARY_INT) {
			e->code = LE_PRIMARY_INT;
			e->tvar->num = 0;
			update_int_view(e);
		}
		return 1;
	}

	if (is_le_not_num(e->l, 0))
		do_opt(paste_le(e, e->r)); // e = e->l
	else if (is_le_not_num(e->r, 0))
		do_opt(paste_le(e, e->l)); // e = e->r
	if (opted) {				   // e -> bool(e)
		turn_to_bool(e);
		return 1;
	}

	return check_if_bools(e);
}

// e || true  -> true
// e || false -> bool(e)
int try_opt_or(struct LocalExpr *e) {
	int opted = 0;

	if (is_le_num(e->l, 0))
		do_opt(paste_le(e, e->r)); // e = e->r
	else if (is_le_num(e->r, 0))
		do_opt(paste_le(e, e->l)); // e = e->l
	if (opted) {				   // e -> bool(e)
		turn_to_bool(e);
		return 1;
	}

	if (is_le_not_num(e->l, 0)) // not check sf cuz lazy eval
		do_opt(paste_le(e, e->l));
	else if (is_le_not_num(e->r, 0) && have_only_gvar_effect_or_none(e->l))
		do_opt(paste_le(e, e->r));
	if (opted) { // e -> true, true is 1, where e is already num
		turn_type_to_i32(e);
		if (e->code != LE_PRIMARY_INT || e->tvar->num != 1) {
			e->code = LE_PRIMARY_INT;
			e->tvar->num = 1;
			update_int_view(e);
		}
		return 1;
	}

	return check_if_bools(e);
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
		if (is_le_num(e->l, 0)) {
			if (have_only_gvar_effect_or_none(e->r))
				do_opt(paste_le(e, e->l));
		} else if (is_le_num(e->l, -1))
			do_opt(paste_le(e, e->r));
		else if (e->l->type) {
			if ((is_u_or_i_32(e->l->type) && is_le_num(e->l, 0xFFFFFFFF)) ||
				(is_u_or_i_16(e->l->type) && is_le_num(e->l, 0xFFFF)) ||
				(is_u_or_i_8(e->l->type) && is_le_num(e->l, 0xFF)))
				do_opt(paste_le(e, e->r));
		}
	} else if (is_num_le(e->r)) {
		if (is_le_num(e->r, 0)) {
			if (have_only_gvar_effect_or_none(e->l))
				do_opt(paste_le(e, e->r));
		} else if (is_le_num(e->r, -1))
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

// e < e  -> false, but if e is not side effective
// e > e  -> false, but if e is not side effective
int try_opt_more_or_less(struct LocalExpr *e) {
	int opted = 0;
	if (both_not_side_effective(e->l, e->r) && lee(e->l, e->r)) {
		opted = 1;
		// free_req(e->l, e->r)
		turn_type_to_i32(e);
		if (e->code != LE_PRIMARY_INT || e->tvar->num != 0) {
			e->code = LE_PRIMARY_INT;
			e->tvar->num = 0;
			update_int_view(e);
		}
	}
	return opted;
}
// e == e -> true,  but if e is not side effective
// e != e -> false, but if e is not side effective
int try_opt_eque_or_nequ(struct LocalExpr *e) {
	int opted = 0;
	if (both_not_side_effective(e->l, e->r) && lee(e->l, e->r)) {
		opted = 1;
		// free_req(e->l, e->r)
		turn_type_to_i32(e);
		if (e->code != LE_PRIMARY_INT) {
			e->tvar->num = lceb(EQUALS);
			e->code = LE_PRIMARY_INT;
			update_int_view(e);
		}
	}
	return opted;
}
// e <= e -> true,  but if e is not side effective
// e >= e -> true,  but if e is not side effective
int try_opt_moree_or_lesse(struct LocalExpr *e) {
	int opted = 0;
	if (both_not_side_effective(e->l, e->r) && lee(e->l, e->r)) {
		opted = 1;
		// free_req(e->l, e->r)
		turn_type_to_i32(e);
		if (e->code != LE_PRIMARY_INT || e->tvar->num != 1) {
			e->code = LE_PRIMARY_INT;
			e->tvar->num = 1;
			update_int_view(e);
		}
	}
	return opted;
}
