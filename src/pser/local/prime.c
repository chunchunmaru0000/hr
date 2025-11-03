#include "../pser.h"

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
			e = after_l_expression(p);                                         \
			unary = new_local_expr((le_code), 0, c);                           \
			unary->l = e;                                                      \
			goto unary_return;                                                 \
		}                                                                      \
	} while (0)

struct LocalExpr *unary_l_expression(struct Pser *p) {
	struct Token *c = pser_cur(p), *last_minus;
	struct LocalExpr *unary, *e;
	char sign = 1;

	if (c->code == PLUS || c->code == MINUS) {
		// LE_UNARY_MINUS
		loop {
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
			goto default_return;

		e = after_l_expression(p);
		unary = new_local_expr(LE_UNARY_MINUS, 0, last_minus);
		unary->l = e;
		goto unary_return;
	}
	// TODO: unary proper parse with optimizations

	// these are hard ones to think
	one_token_unary(MUL, LE_UNARY_ADDR);
	one_token_unary(AMPER, LE_UNARY_AMPER);

	// can anigilate itself on itself
	one_token_unary(BIT_NOT, LE_UNARY_BIT_NOT);
	// can anigilate itself on itself, but if anigilate then its LE_BOOL
	one_token_unary(EXCL, LE_UNARY_NOT);

	// these are good
	one_token_unary(INC, LE_UNARY_INC);
	one_token_unary(DEC, LE_UNARY_DEC);

default_return:
	return after_l_expression(p);
unary_return:
	return unary;
}
