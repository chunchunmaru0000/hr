#include "pser.h"

// ### os explanation
// ... - Arg's
enum IP_Code pser_local_inst_let(struct Pser *p, struct PList *os) {
	expect(p, absorb(p), PAR_L);
	parse_args(p, os);
	// TODO check here for identical names and with fun arg tohether?

	return IP_LET;
}

// ### os explanation
//   _ - label name token
enum IP_Code pser_local_inst_label(struct Pser *p, struct PList *os,
								   struct Token *name) {
	plist_add(os, name);
	consume(p); // consume label name
	consume(p); // consume :
	return IP_DECLARE_LABEL;
}

// ### os explanation
//   _ - label name token
enum IP_Code pser_local_inst_goto(struct Pser *p, struct PList *os) {
	struct Token *name = absorb(p);
	match(p, name, ID);
	plist_add(os, name);
	return IP_GOTO;
}

// ### os explanation
// ... - local instructions
enum IP_Code pser_local_inst_loop(struct Pser *p, struct PList *os) {
	consume(p); // consime вечно
	parse_block_of_local_inst(p, os);
	return IP_LOOP;
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
		else if (sc(cv, STR_GOTO))
			code = pser_local_inst_goto(p, os);

		else if (n->code == COLO)
			code = pser_local_inst_label(p, os, c);
		else if (sc(cv, STR_LOOP))
			code = pser_local_inst_loop(p, os);

		if (code != IP_NONE)
			break;
	default:
		eet(p->f, c, "ээээ ты че", 0);
	}
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
	// 	IP_IF_ELIF_ELSE,
	// 	IP_WHILE_LOOP,
	// 	IP_FOR_LOOP,
	//
	// 	IP_MATCH,

	return new_inst(p, code, os, c);
}
