#include "pser.h"

// ### os explanation
// ... - Arg's
enum IP_Code pser_local_inst_let(struct Pser *p, struct PList *os) {
	expect(p, absorb(p), PAR_L);
	parse_args(p, os);

	return IP_LET;
}

struct Inst *get_local_inst(struct Pser *p) {
	struct Token *c = pser_cur(p), *n;
	struct PList *os = new_plist(2);
	char *cv = (char *)c->view->st;
	enum IP_Code code = IP_NONE;

	while (c->code == SLASHN || c->code == SEP)
		c = absorb(p);
	n = get_pser_token(p, 1);

	// fill *os in funcs
	switch (c->code) {
	case EF:
		code = IP_EOI;
		break;
	case ID:
		if (sc(cv, STR_ASM))
			code = inst_pser_asm(p, os);
		else if (sc(cv, STR_LET))
			code = pser_local_inst_let(p, os);

		if (code != IP_NONE)
			break;
	default:
		eet(p->f, c, ERR_WRONG_TOKEN, 0);
	}
	// 	IP_LET,
	//
	// 	IP_DECLARE_LABEL,
	// 	IP_GOTO,
	//
	// 	expression,
	//
	// 	IP_EQU,
	// 	IP_PLUS_EQU,
	// 	IP_MINUS_EQU,
	// 	IP_MUL_EQU,
	// 	IP_DIV_EQU,
	// 	IP_SHR_EQU,
	// 	IP_SHL_EQU,
	//
	// 	IP_LOOP,
	//
	// 	IP_IF_ELIF_ELSE,
	// 	IP_WHILE_LOOP,
	// 	IP_FOR_LOOP,
	//
	// 	IP_MATCH, // TODO: I_MATCH

	return new_inst(p, code, os, c);
}
