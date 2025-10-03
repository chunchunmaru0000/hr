#include "../pser.h"

struct NamedStructField *new_field(struct Token *name, struct GlobExpr *e) {
	struct NamedStructField *field = malloc(sizeof(struct NamedStructField));
	field->name_token = name;
	field->expression = e;
	return field;
}

void global_single_struct(struct Pser *p, struct GlobExpr *e, struct Token *c) {
	e->code = CT_STRUCT_PTR;
	free(e->tvar); // it was malloced above
	e->tvar = c;   // HERE TOKEN IS NOT COPIED
	e->globs = new_plist(4);

	c = absorb(p); // absorb '{'

	if (c->code == ID && pser_next(p)->code == EQU) {
		e->struct_with_fields = 1;

		goto skip_check_on_is_field;
		for (; not_ef_and(PAR_T_R, c);) {
			expect(c, ID);
			expect(pser_next(p), EQU);
		skip_check_on_is_field:

			consume(p); // skip field name ID token
			consume(p); // skip '='
			plist_add(e->globs, new_field(c, global_expression(p)));

			c = pser_cur(p);
			if (c->code == COMMA) // delimeter
				c = absorb(p);
		}
	} else {
		e->struct_with_fields = 0;

		for (; not_ef_and(PAR_T_R, c);) {
			plist_add(e->globs, global_expression(p));

			c = pser_cur(p);
			if (c->code == COMMA) // delimeter
				c = absorb(p);
		}
	}
	consume(p); // consume PAR_T_R
}
