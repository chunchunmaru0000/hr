#include "../gner/gner.h"

#define causes_side_effects(e)                                                 \
	((e)->code == LE_UNARY_DEC || (e)->code == LE_UNARY_INC ||                 \
	 (e)->code == LE_AFTER_DEC || (e)->code == LE_AFTER_INC ||                 \
	 (e)->code == LE_AFTER_CALL || (e)->code == LE_BIN_ASSIGN ||               \
	 (e)->code == LE_IF_ELSE)

void try_cut_even_when_side_effects(struct PList *es, struct LocalExpr *e) {
	if (lceb(ASSIGN) && both_not_side_effective(e->l, e->r) && lee(e->l, e->r))
		return;

	plist_add(es, e);
}

void cut(struct PList *es, struct LocalExpr *e) {
	if (causes_side_effects(e)) {
	when_side_effects:
		try_cut_even_when_side_effects(es, e);
	} else if (is_primary(e)) {
		;
	} else if (is_unary(e) || lce(BOOL) || lcea(FIELD_OF_PTR) || lcea(FIELD)) {
		cut(es, e->l);
	} else if (is_bin_le(e) || lcea(INDEX)) {
		cut(es, e->l);
		cut(es, e->r);
	} else if (lceb(TERRY)) {
		if (causes_side_effects(e->l) || causes_side_effects(e->r))
			goto when_side_effects;

		cut(es, e->l);
		cut(es, e->r);
		cut(es, e->co.cond);
	}
}

struct PList *eliminate_dead_code_from_le(struct LocalExpr *e) {
	struct PList *es = new_plist(1);

	//gen_tuple_of(ogner, e); TODO: why was it here
	cut(es, e);

	return es;
}
