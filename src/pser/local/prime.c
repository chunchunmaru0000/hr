#include "../pser.h"

#define set_e_code_and_absorb(code_to_set)                                     \
	do {                                                                       \
		e->code = code_to_set;                                                 \
		absorb(p);                                                             \
	} while (0)

struct LocalExpr *prime_l_expression(struct Pser *p) {
	struct Token *c = pser_cur(p);

	struct LocalExpr *e = new_local_expr(LE_NONE, 0, c);

	if (c->code == INT)
		set_e_code_and_absorb(LE_PRIMARY_INT);
	else if (c->code == REAL)
		set_e_code_and_absorb(LE_PRIMARY_REAL);
	else if (c->code == STR)
		set_e_code_and_absorb(LE_PRIMARY_STR);
	else if (c->code == ID)
		set_e_code_and_absorb(LE_PRIMARY_VAR);
	else if (c->code == PAR_L) {
		absorb(p);
		free(e);
		e = local_expression(p);
		match(pser_cur(p), PAR_R);
	} else
		eet(c, "эээ че за выражение", 0);

	return e;
}
