#include "../pser.h"
#include <stdio.h>

#define set_e_code_and_consume(code_to_set)                                    \
	do {                                                                       \
		e->code = code_to_set;                                                 \
		consume(p);                                                            \
	} while (0)

struct LocalExpr *prime_l_expression(struct Pser *p) {
	struct Token *c = pser_cur(p);
	enum TCode ccode = c->code;

	struct LocalExpr *e = new_local_expr(LE_NONE, 0, c), *tmp_e;

	if (ccode == INT)
		set_e_code_and_consume(LE_PRIMARY_INT);
	else if (ccode == REAL)
		set_e_code_and_consume(LE_PRIMARY_REAL);
	else if (ccode == STR)
		set_e_code_and_consume(LE_PRIMARY_STR);
	else if (ccode == ID)
		set_e_code_and_consume(LE_PRIMARY_VAR);
	else if (ccode == PAR_C_L) {
		absorb(p);
		e->code = LE_PRIMARY_ARR;
		e->co.ops = new_plist(2);
		for (c = pser_cur(p); c->code != PAR_C_R;) {
			plist_add(e->co.ops, local_expression(p));
			if ((c = pser_cur(p))->code == COMMA)
				c = absorb(p);
		}
		consume(p); // skip ]
	} else if (ccode == PAR_L) {

		if ((c = absorb(p))->code == PAR_R) {		  // skip '('
			set_e_code_and_consume(LE_PRIMARY_TUPLE); // skip ')'
			e->co.ops = new_plist(1);
			goto return_e;
		}

		tmp_e = local_expression(p);
		c = pser_cur(p);

		if (c->code == COMMA) {
			e->code = LE_PRIMARY_TUPLE;
			e->co.ops = new_plist(2);
			plist_add(e->co.ops, tmp_e);

			for (; c->code == COMMA; c = pser_cur(p)) {
				c = absorb(p);
				if (c->code == PAR_R)
					break;
				plist_add(e->co.ops, local_expression(p));
			}
		} else {
			free(e);
			e = tmp_e;
		}

		match(pser_cur(p), PAR_R);
	} else
		eet(c, "эээ че за выражение", 0);

return_e:
	return e;
}

#define one_token_unary(c_code, le_code)                                       \
	do {                                                                       \
		if (c->code == (c_code)) {                                             \
			consume(p);                                                        \
			e = unary_l_expression(p);                                         \
			unary = new_local_expr((le_code), 0, c);                           \
			unary->l = e;                                                      \
			return unary;                                                      \
		}                                                                      \
	} while (0)
#define annihilate_one_on_other(ccode, one, other)                             \
	do {                                                                       \
		if (c->code == (ccode)) {                                              \
			consume(p);                                                        \
			e = unary_l_expression(p);                                         \
			if (lceu(one)) {                                                   \
				unary = e->l;                                                  \
				free(e);                                                       \
			} else {                                                           \
				unary = new_local_expr(LE_UNARY_##other, 0, c);                \
				unary->l = e;                                                  \
			}                                                                  \
			return unary;                                                      \
		}                                                                      \
	} while (0)

struct LocalExpr *unary_l_expression(struct Pser *p) {
	struct Token *c = pser_cur(p), *last_minus;
	struct LocalExpr *unary, *e;
	char sign = 1;

	if (c->code == PLUS || c->code == MINUS) {
		// LE_UNARY_MINUS
		forever {
			if (c->code == PLUS) {
				c = absorb(p);
				continue;
			}
			if (c->code == MINUS) {
				last_minus = c;
				c = absorb(p);
				sign *= -1;
				continue;
			}
			break;
		}
		if (sign == 1)
			goto unary_expression_return;

		e = unary_l_expression(p);
		unary = new_local_expr(LE_UNARY_MINUS, 0, last_minus);
		unary->l = e;
		return unary;
	}
	// these are annihilate one on other, dont care more in here(parser)
	annihilate_one_on_other(MUL, AMPER, ADDR);
	annihilate_one_on_other(AMPER, ADDR, AMPER);

	// can annihilate itself on itself
	annihilate_one_on_other(BIT_NOT, BIT_NOT, BIT_NOT);

	// can annihilate itself on itself, but if annihilate then its LE_BOOL
	if (c->code == EXCL) {
		consume(p);
		e = unary_l_expression(p);
		if (lceu(NOT)) {
			(unary = e)->code = LE_BOOL;
		} else if (lce(BOOL)) {
			(unary = e)->code = LE_UNARY_NOT;
		} else {
			unary = new_local_expr(LE_UNARY_NOT, 0, c);
			unary->l = e;
		}
		return unary;
	}

	// these are good
	one_token_unary(INC, LE_UNARY_INC);
	one_token_unary(DEC, LE_UNARY_DEC);

	if (c->code == ID) {
		if (sc(vs(c), STR_SIZE_OF)) {
			consume(p);
			unary = new_local_expr(LE_SIZE_OF, 0, c);
			unary->l = (void *)type_expr(p);
		} else if (sc(vs(c), STR_SIZE_OF_VAL)) {
			consume(p);
			unary = new_local_expr(LE_SIZE_OF_VAL, 0, c);
			unary->l = local_expression(p);
		} else if (sc(vs(c), STR_AS)) {
			consume(p);
			unary = new_local_expr(LE_AS, 0, c);
			unary->l = (void *)type_expr(p);
			unary->r = local_expression(p);
		} else
			goto default_return;
		return unary;
	}

default_return:
	return after_l_expression(p);
unary_expression_return:
	return unary_l_expression(p);
}
