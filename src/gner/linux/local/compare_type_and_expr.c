#include "../../gner.h"

constr VAR_TYPE_NOT_FOUND = "Тип переменной не определен.";

void (*cmps[])(struct TypeExpr *type, struct LocalExpr *e) = {
	0, // LE_NONE = 0,
	0, // LE_PRIMARY_INT = 1,
	0, // LE_PRIMARY_REAL = 2,
	0, // LE_PRIMARY_VAR = 3,
	0, // LE_PRIMARY_STR = 4,
	0, // LE_PRIMARY_ARR = 5,
	0, // LE_PRIMARY_TUPLE = 6,
	0, // LE_PRIMARY_INDEX = 7,
	0, // LE_PRIMARY_CALL = 8,
	0, // LE_PRIMARY_FIELD_OF_PTR = 9,
	0, // LE_PRIMARY_FIELD = 10,
	0, // LE_PRIMARY_INC_AFTER = 11,
	0, // LE_PRIMARY_DEC_AFTER = 12,
	0, // LE_UNARY_MINUS = 13,
	0, // LE_UNARY_INC_BEFORE = 14,
	0, // LE_UNARY_DEC_BEFORE = 15,
	0, // LE_UNARY_NOT = 16,
	0, // LE_UNARY_BIT_NOT = 17,
	0, // LE_UNARY_AMPER = 18,
	0, // LE_UNARY_ADDR = 19,
	0, // LE_BIN_MUL = 20,
	0, // LE_BIN_DIV = 21,
	0, // LE_BIN_MOD = 22,
	0, // LE_BIN_PLUS = 23,
	0, // LE_BIN_MINUS = 24,
	0, // LE_BIN_SHL = 25,
	0, // LE_BIN_SHR = 26,
	0, // LE_BIN_LESS = 27,
	0, // LE_BIN_LESSE = 28,
	0, // LE_BIN_MORE = 29,
	0, // LE_BIN_MOREE = 30,
	0, // LE_BIN_EQUALS = 31,
	0, // LE_BIN_NOT_EQUALS = 32,
	0, // LE_BIN_BIT_AND = 33,
	0, // LE_BIN_BIT_XOR = 34,
	0, // LE_BIN_BIT_OR = 35,
	0, // LE_BIN_AND = 36,
	0, // LE_BIN_OR = 37,
	0, // LE_BIN_TERRY = 38,
	0, // LE_BIN_ASSIGN = 39,
	0, // LE_BIN_PIPE_LINE = 40,
	0, // LE_AFTER_CALL = 41,
	0, // LE_AFTER_FIELD_OF_PTR = 42,
	0, // LE_AFTER_FIELD = 43,
};
void compare_type_and_expr(struct TypeExpr *type, struct LocalExpr *e) {
	void (*cmp)(struct TypeExpr *type, struct LocalExpr *e) = cmps[e->code];

	if (type == 0)
		eet(e->tvar, VAR_TYPE_NOT_FOUND, 0);
	if (cmp == 0)
		eet(e->tvar, "эээ 123", 0);
	cmp(type, e);
}
