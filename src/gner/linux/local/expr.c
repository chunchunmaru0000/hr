#include "../../gner.h"
#include <stdio.h>

sa(STR_XOR_EAX_EAX, "искл еах еах");
sa(MOV, "быть ");
sa(PAR_RBP, "(рбп ");
sa(R_PAR, ") ");
sa(BYTE, "байт ");
sa(WORD, "дбайт ");
sa(DWORD, "чбайт ");
sa(QWORD, "вбайт ");

struct BList *size_str(uc size) {
	if (size == BYTE)
		return copy_blist_from_str((char *)SA_BYTE);
	if (size == WORD)
		return copy_blist_from_str((char *)SA_WORD);
	if (size == DWORD)
		return copy_blist_from_str((char *)SA_DWORD);
	if (size == QWORD)
		return copy_blist_from_str((char *)SA_QWORD);
	exit(127);
}

void gen_int(struct Gner *g, struct LocalExpr *e) {
	long num = e->tvar->num;

	if (num == 0) {
		iprint_fun_text(SA_STR_XOR_EAX_EAX);
	} else {
	}
}

void gen_real(struct Gner *g, struct LocalExpr *e);
void gen_var(struct Gner *g, struct LocalExpr *e);
void gen_assign(struct Gner *g, struct LocalExpr *e);

void (*gen_expressions[])(struct Gner *g, struct LocalExpr *e) = {
	0,			//		LE_NONE
	gen_int,	// 	LE_PRIMARY_INT,
	gen_real,	// 	LE_PRIMARY_REAL,
	gen_var,	// 	LE_PRIMARY_VAR,
	0,			// 	LE_PRIMARY_STR,
	0,			// 	LE_PRIMARY_ARR,
	0,			// 	LE_PRIMARY_TUPLE,
	0,			// 	LE_PRIMARY_INDEX,
	0,			// 	LE_PRIMARY_CALL,
	0,			// 	LE_PRIMARY_FIELD_OF_PTR,
	0,			// 	LE_PRIMARY_FIELD,
	0,			// 	LE_PRIMARY_INC_AFTER,
	0,			// 	LE_PRIMARY_DEC_AFTER,
	0,			// 	LE_UNARY_MINUS,
	0,			// 	LE_UNARY_INC_BEFORE,
	0,			// 	LE_UNARY_DEC_BEFORE,
	0,			// 	LE_UNARY_NOT,
	0,			// 	LE_UNARY_BIT_NOT,
	0,			// 	LE_UNARY_AMPER,
	0,			// 	LE_UNARY_ADDR,
	0,			// 	LE_BIN_MUL,
	0,			// 	LE_BIN_DIV,
	0,			// 	LE_BIN_MOD,
	0,			// 	LE_BIN_PLUS,
	0,			// 	LE_BIN_MINUS,
	0,			// 	LE_BIN_SHL,
	0,			// 	LE_BIN_SHR,
	0,			// 	LE_BIN_LESS,
	0,			// 	LE_BIN_LESSE,
	0,			// 	LE_BIN_MORE,
	0,			// 	LE_BIN_MOREE,
	0,			// 	LE_BIN_EQUALS,
	0,			// 	LE_BIN_NOT_EQUALS,
	0,			// 	LE_BIN_BIT_AND,
	0,			// 	LE_BIN_BIT_XOR,
	0,			// 	LE_BIN_BIT_OR,
	0,			// 	LE_BIN_AND,
	0,			// 	LE_BIN_OR,
	0,			// 	LE_BIN_TERRY,
	gen_assign, // 	LE_BIN_ASSIGN,
	0,			// 	LE_BIN_PIPE_LINE,
	0,			// 	LE_AFTER_CALL,
	0,			// 	LE_AFTER_FIELD_OF_PTR,
	0,			// 	LE_AFTER_FIELD,
};

void gen_local_expression_linux(struct Gner *g, struct Inst *in) {
	struct LocalExpr *e = plist_get(in->os, 0);

	if (gen_expressions[e->code] == 0) {
		printf("### GEN LOCAL EXPR INFO: e->code == %d\n", e->code);
		return;
	}

	free_all_regs(g->cpu);
	gen_expressions[e->code](g, e);
}

void gen_real(struct Gner *g, struct LocalExpr *e) {}

void gen_var(struct Gner *g, struct LocalExpr *e) {}

void gen_assign(struct Gner *g, struct LocalExpr *e) {
	struct LocalExpr *assignee = plist_get(e->ops, 0);
	struct LocalExpr *assignable = plist_get(e->ops, 1);
	struct Reg *rax, *reg;

	printf("### GEN assignee INFO: assignee->code == %d\n", assignee->code);
	printf("### GEN assignable INFO: assignable->code == %d\n",
		   assignable->code);

	uc assignee_size = get_assignee_size(g, assignee);

	if (assignee->code == LE_BIN_ASSIGN) {
	} else if (assignee->code == LE_PRIMARY_VAR) {
		if (assignable->code == LE_PRIMARY_INT) {
			iprint_fun_text(SA_MOV);						  // быть
			blat_blist(g->fun_text, size_str(assignee_size)); // *байт
			print_fun_text(SA_PAR_RBP);						  // (рбп
			blat_blist(g->fun_text, assignee->tvar->view);	  // перем
			blat_str_fun_text(SA_R_PAR);					  // )
			int_add(g->fun_text, assignable->tvar->num);	  // число
			blat_str_fun_text(SA_START_COMMENT);			  // ;
			hex_int_add(g->fun_text, assignable->tvar->num);  // х число
			fun_text_add('\n');								  // \n
		} else if (assignable->code == LE_PRIMARY_REAL) {
		} else {
		}
	}

	// rax = try_borrow_reg(e->tvar, g->cpu, QWORD);
	//  reg = try_borrow_reg(e->tvar, g->cpu, );
	//   if (assignable->code == LE_BIN_ASSIGN) {
	//   	gen_to_reg(g, rax, assignable);
	//   	gen_to_reg(g, reg, assignable);
	//   }
}
