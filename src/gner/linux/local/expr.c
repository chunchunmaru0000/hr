#include "../../gner.h"
#include <stdio.h>

sa(STR_XOR_EAX_EAX, "искл еах еах");

void gen_int(struct Gner *g, struct LocalExpr *e) {
	long num = e->tvar->num;

	if (num == 0) {
		iprint_fun_text(SA_STR_XOR_EAX_EAX);
	} else {
	}
}
void gen_real(struct Gner *g, struct LocalExpr *e) {}
void gen_var(struct Gner *g, struct LocalExpr *e) {}

void (*gen_expressions[])(struct Gner *g, struct LocalExpr *e) = {
	0,		  //		LE_NONE
	gen_int,  // 	LE_PRIMARY_INT,
	gen_real, // 	LE_PRIMARY_REAL,
	gen_var,  // 	LE_PRIMARY_VAR,
	0,		  // 	LE_PRIMARY_STR,
	0,		  // 	LE_PRIMARY_ARR,
	0,		  // 	LE_PRIMARY_TUPLE,
	0,		  // 	LE_PRIMARY_INDEX,
	0,		  // 	LE_PRIMARY_CALL,
	0,		  // 	LE_PRIMARY_FIELD_OF_PTR,
	0,		  // 	LE_PRIMARY_FIELD,
	0,		  // 	LE_PRIMARY_INC_AFTER,
	0,		  // 	LE_PRIMARY_DEC_AFTER,
	0,		  // 	LE_UNARY_MINUS,
	0,		  // 	LE_UNARY_INC_BEFORE,
	0,		  // 	LE_UNARY_DEC_BEFORE,
	0,		  // 	LE_UNARY_NOT,
	0,		  // 	LE_UNARY_BIT_NOT,
	0,		  // 	LE_UNARY_AMPER,
	0,		  // 	LE_UNARY_ADDR,
	0,		  // 	LE_BIN_MUL,
	0,		  // 	LE_BIN_DIV,
	0,		  // 	LE_BIN_MOD,
	0,		  // 	LE_BIN_PLUS,
	0,		  // 	LE_BIN_MINUS,
	0,		  // 	LE_BIN_SHL,
	0,		  // 	LE_BIN_SHR,
	0,		  // 	LE_BIN_LESS,
	0,		  // 	LE_BIN_LESSE,
	0,		  // 	LE_BIN_MORE,
	0,		  // 	LE_BIN_MOREE,
	0,		  // 	LE_BIN_EQUALS,
	0,		  // 	LE_BIN_NOT_EQUALS,
	0,		  // 	LE_BIN_BIT_AND,
	0,		  // 	LE_BIN_BIT_XOR,
	0,		  // 	LE_BIN_BIT_OR,
	0,		  // 	LE_BIN_AND,
	0,		  // 	LE_BIN_OR,
	0,		  // 	LE_BIN_TERRY,
	0,		  // 	LE_BIN_ASSIGN,
	0,		  // 	LE_BIN_PIPE_LINE,
	0,		  // 	LE_AFTER_CALL,
	0,		  // 	LE_AFTER_FIELD_OF_PTR,
	0,		  // 	LE_AFTER_FIELD,
};

void gen_local_expression_linux(struct Gner *g, struct Inst *in) {
	struct LocalExpr *e = plist_get(in->os, 0);

	if (gen_expressions[e->code] == 0) {
		printf("### GEN LOCAL EXPR INFO: e->code == %d\n", e->code);
		return;
	}

	gen_expressions[e->code](g, e);
}
