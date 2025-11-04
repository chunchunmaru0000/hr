#include "../../gner.h"

constr VAR_TYPE_NOT_FOUND = "Тип переменной не определен.";
constr EXPR_TYPE_NOT_VALID_FOR_TYPE[] = {
	0,												// LE_NONE
	"Неверный целый числовой тип выражения.",		// LE_PRIMARY_INT
	"Неверный вещественый числовой тип выражения.", // LE_PRIMARY_REAL
	"Неверный вещественый числовой тип выражения.", // LE_PRIMARY_VA
	0,												// LE_PRIMARY_STR
	0,												// LE_PRIMARY_ARR
	0,												// LE_PRIMARY_TUPLE
	0,												// LE_UNARY_MINUS
	0,												// LE_UNARY_INC
	0,												// LE_UNARY_DEC
	0,												// LE_UNARY_NOT
	0,												// LE_UNARY_BIT_NOT
	0,												// LE_UNARY_AMPER
	0,												// LE_UNARY_ADDR
	0,												// LE_BIN_MUL
	0,												// LE_BIN_DIV
	0,												// LE_BIN_MOD
	0,												// LE_BIN_WHOLE_DIV
	0,												// LE_BIN_ADD
	0,												// LE_BIN_SUB
	0,												// LE_BIN_SHL
	0,												// LE_BIN_SHR
	0,												// LE_BIN_LESS
	0,												// LE_BIN_LESSE
	0,												// LE_BIN_MORE
	0,												// LE_BIN_MOREE
	0,												// LE_BIN_EQUALS
	0,												// LE_BIN_NOT_EQUALS
	0,												// LE_BIN_BIT_AND
	0,												// LE_BIN_BIT_XOR
	0,												// LE_BIN_BIT_OR
	0,												// LE_BIN_AND
	0,												// LE_BIN_OR
	0,												// LE_BIN_TERRY
	0,												// LE_BIN_ASSIGN
	0,												// LE_AFTER_PIPE_LINE,
	0,												// LE_AFTER_INDEX,
	0,												// LE_AFTER_CALL,
	0,												// LE_AFTER_INC,
	0,												// LE_AFTER_DEC,
	0,												// LE_AFTER_FIELD_OF_PTR,
	0,												// LE_AFTER_FIELD,
	0,												// LE_AFTER_ENUM,
	0,												// LE_BOOL,
};

void cmp_int(struct TypeExpr *type, struct LocalExpr *e) {
	if (is_num_int_type(type))
		return;
	else if (is_real_type(type)) {
		e->tvar->real = e->tvar->num;
		e->code = LE_PRIMARY_REAL;
	} else if (type->code == TC_ARR)
		compare_type_and_expr(arr_type(type), e);
	else
		eet(e->tvar, EXPR_TYPE_NOT_VALID_FOR_TYPE[e->code], 0);
}

void cmp_real(struct TypeExpr *type, struct LocalExpr *e) {
	if (is_real_type(type))
		return;
	else if (is_num_int_type(type)) {
		e->tvar->num = e->tvar->real;
		e->code = LE_PRIMARY_INT;
	} else if (type->code == TC_ARR)
		compare_type_and_expr(arr_type(type), e);
	else
		eet(e->tvar, EXPR_TYPE_NOT_VALID_FOR_TYPE[e->code], 0);
}

void (*cmps[])(struct TypeExpr *type, struct LocalExpr *e) = {
	0,		  // LE_NONE
	cmp_int,  // LE_PRIMARY_INT
	cmp_real, // LE_PRIMARY_REAL
	0,		  // LE_PRIMARY_VAR
	0,		  // LE_PRIMARY_STR
	0,		  // LE_PRIMARY_ARR
	0,		  // LE_PRIMARY_TUPLE
	0,		  // LE_UNARY_MINUS
	0,		  // LE_UNARY_INC
	0,		  // LE_UNARY_DEC
	0,		  // LE_UNARY_NOT
	0,		  // LE_UNARY_BIT_NOT
	0,		  // LE_UNARY_AMPER
	0,		  // LE_UNARY_ADDR
	0,		  // LE_BIN_MUL
	0,		  // LE_BIN_DIV
	0,		  // LE_BIN_MOD
	0,		  // LE_BIN_WHOLE_DIV
	0,		  // LE_BIN_ADD
	0,		  // LE_BIN_SUB
	0,		  // LE_BIN_SHL
	0,		  // LE_BIN_SHR
	0,		  // LE_BIN_LESS
	0,		  // LE_BIN_LESSE
	0,		  // LE_BIN_MORE
	0,		  // LE_BIN_MOREE
	0,		  // LE_BIN_EQUALS
	0,		  // LE_BIN_NOT_EQUALS
	0,		  // LE_BIN_BIT_AND
	0,		  // LE_BIN_BIT_XOR
	0,		  // LE_BIN_BIT_OR
	0,		  // LE_BIN_AND
	0,		  // LE_BIN_OR
	0,		  // LE_BIN_TERRY
	0,		  // LE_BIN_ASSIGN
	0,		  // LE_AFTER_PIPE_LINE,
	0,		  // LE_AFTER_INDEX,
	0,		  // LE_AFTER_CALL,
	0,		  // LE_AFTER_INC,
	0,		  // LE_AFTER_DEC,
	0,		  // LE_AFTER_FIELD_OF_PTR,
	0,		  // LE_AFTER_FIELD,
	0,		  // LE_AFTER_ENUM,
	0,		  // LE_BOOL,
};
void compare_type_and_expr(struct TypeExpr *type, struct LocalExpr *e) {
	void (*cmp)(struct TypeExpr *type, struct LocalExpr *e) = cmps[e->code];

	if (type == 0)
		eet(e->tvar, VAR_TYPE_NOT_FOUND, 0);
	if (cmp)
		cmp(type, e);
}
