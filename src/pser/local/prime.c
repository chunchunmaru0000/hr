#include "../pser.h"

#define set_e_code_and_absorb(code_to_set)                                     \
	do {                                                                       \
		e->code = code_to_set;                                                 \
		absorb(p);                                                             \
	} while (0)

struct LocalExpr *prime_l_expression(struct Pser *p) {
	struct Token *c = pser_cur(p);
	enum TCode ccode = c->code;

	struct LocalExpr *e = new_local_expr(LE_NONE, 0, c), *tmp_e;

	if (ccode == INT)
		set_e_code_and_absorb(LE_PRIMARY_INT);
	else if (ccode == REAL)
		set_e_code_and_absorb(LE_PRIMARY_REAL);
	else if (ccode == STR)
		set_e_code_and_absorb(LE_PRIMARY_STR);
	else if (ccode == ID)
		set_e_code_and_absorb(LE_PRIMARY_VAR);
	else if (ccode == PAR_L) {
		absorb(p);
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
				tmp_e = local_expression(p);
				plist_add(e->co.ops, tmp_e);
			}
		} else {
			free(e);
			e = tmp_e;
		}

		match(pser_cur(p), PAR_R);
	} else
		eet(c, "эээ че за выражение", 0);

	return e;
}
