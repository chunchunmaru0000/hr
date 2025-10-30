#include "../gner/gner.h"

constr EXPECTED_ARR_TYPE =
	"Ожидалось выражение типа массива для обращения к нему по индексу.";

void define_var_type(struct LocalExpr *e) {
	struct LocalVar *lvar;
	struct GlobVar *gvar;
}

void define_le_type(struct LocalExpr *e) {
	if (e->type)
		return;

	enum LE_Code code = e->code;
	u32 i;

	if (is_bin_le(e)) {
		define_le_type(e->l);
		define_le_type(e->r);

	} else if (code == LE_BIN_TERRY) {
		define_le_type(e->l);
		define_le_type(e->r);
		define_le_type(e->co.cond);

	} else if (code == LE_PRIMARY_INT) {
		e->type = new_type_expr(TC_I32);
	} else if (code == LE_PRIMARY_REAL) {
		e->type = new_type_expr(TC_DOUBLE);
	} else if (code == LE_PRIMARY_VAR) {
		define_var_type(e);
	} else if (code == LE_PRIMARY_STR || code == LE_PRIMARY_ARR ||
			   code == LE_PRIMARY_TUPLE) {
		e->type = 0;
	} else if (code == LE_AFTER_INDEX) {
		define_le_type(e->l);
		define_le_type(e->r);
		if (e->l->type->code != TC_ARR)
			eet(e->l->tvar, EXPECTED_ARR_TYPE, 0);
		e->type = copy_type_expr(arr_type(e->l->type));
	} else if (code == LE_AFTER_CALL) {
		define_le_type(e->l);
		for (i = 0; i < e->co.ops->size; i++)
			define_le_type(plist_get(e->co.ops, i));
		e->type = copy_type_expr(arr_type(e->l->type));
		// LE_AFTER_INC
		// LE_AFTER_DEC
		// LE_UNARY_MINUS
		// LE_UNARY_INC_BEFORE
		// LE_UNARY_DEC_BEFORE
		// LE_UNARY_NOT
		// LE_UNARY_BIT_NOT
		// LE_UNARY_AMPER
		// LE_UNARY_ADDR
	} else if (code == LE_BIN_ASSIGN) {

	} else if (code == LE_AFTER_PIPE_LINE) {

	} else if (code == LE_AFTER_FIELD_OF_PTR) {
	}
}
