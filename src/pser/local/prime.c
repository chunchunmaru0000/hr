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

	if (ccode == INT) {
		set_e_code_and_absorb(LE_PRIMARY_INT);
		e->type = new_type_expr(TC_I32);
	} else if (ccode == REAL) {
		set_e_code_and_absorb(LE_PRIMARY_REAL);
		e->type = new_type_expr(TC_DOUBLE);
	} else if (ccode == STR) {
		set_e_code_and_absorb(LE_PRIMARY_STR);
		e->type = new_type_expr(TC_PTR);
		e->type->data.ptr_target = new_type_expr(TC_U8);
	} else if (ccode == ID)
		set_e_code_and_absorb(LE_PRIMARY_VAR);
	else if (ccode == PAR_C_L) {
		absorb(p);
		e->code = LE_PRIMARY_ARR;
		e->co.ops = new_plist(2);

	} else if (ccode == PAR_L) {
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
