#include "../gner/gner.h"

/*
	LE_PRIMARY_INT = 1,
	LE_PRIMARY_REAL = 2,
	LE_PRIMARY_VAR = 3,
	LE_PRIMARY_STR = 4,
	LE_PRIMARY_ARR = 5,
	LE_PRIMARY_TUPLE = 6,
	LE_PRIMARY_INDEX = 7,
	LE_PRIMARY_CALL = 8,
	LE_PRIMARY_FIELD_OF_PTR = 9,
	LE_PRIMARY_FIELD = 10,
	LE_PRIMARY_INC_AFTER = 11,
	LE_PRIMARY_DEC_AFTER = 12,
	LE_UNARY_MINUS = 13,
	LE_UNARY_INC_BEFORE = 14,
	LE_UNARY_DEC_BEFORE = 15,
	LE_UNARY_NOT = 16,
	LE_UNARY_BIT_NOT = 17,
	LE_UNARY_AMPER = 18,
	LE_UNARY_ADDR = 19,
	LE_BIN
	LE_BIN_TERRY = 39,
	LE_BIN_ASSIGN = 40,
	LE_BIN_PIPE_LINE = 41,
	LE_AFTER_CALL = 42,
	LE_AFTER_FIELD_OF_PTR = 43,
	LE_AFTER_FIELD = 44,
*/

void define_var_type(struct LocalExpr *e) {
	struct LocalVar *lvar;
	struct GlobVar *gvar;
}

void define_le_type(struct LocalExpr *e) {
	if (e->type)
		return;

	enum LE_Code code = e->code;

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
	} else if (code == LE_PRIMARY_INDEX) {

		// LE_PRIMARY_CALL = 8,
		// LE_PRIMARY_FIELD_OF_PTR = 9,
		// LE_PRIMARY_FIELD = 10,
		// LE_PRIMARY_INC_AFTER = 11,
		// LE_PRIMARY_DEC_AFTER = 12,
		// LE_UNARY_MINUS = 13,
		// LE_UNARY_INC_BEFORE = 14,
		// LE_UNARY_DEC_BEFORE = 15,
		// LE_UNARY_NOT = 16,
		// LE_UNARY_BIT_NOT = 17,
		// LE_UNARY_AMPER = 18,
		// LE_UNARY_ADDR = 19,
	} else if (code == LE_BIN_ASSIGN) {

	} else if (code == LE_BIN_PIPE_LINE) {

	} else if (code == LE_AFTER_CALL) {

	} else if (code == LE_AFTER_FIELD_OF_PTR) {

	} else if (code == LE_AFTER_FIELD) {
	}
}
