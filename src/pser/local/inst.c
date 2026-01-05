#include "../pser.h"
#include <stdio.h>

constr NO_LOOP_TO_ = "Использование слова прерывания цикла вне цикла.";
constr USE_INSIDE_LOOP = "использовать слово внутри цикла";

// ### os explanation
// ... - Arg's
enum IP_Code pser_local_inst_let(struct Pser *p, struct PList *os) {
	struct Token *arg_name;
	struct Arg *arg;
	uint32_t i, j;

	expect(absorb(p), PAR_L);
	parse_args(p, os);

	for (i = 0; i < os->size; i++) {
		arg = plist_get(os, i);

		for (j = 0; j < arg->names->size; j++) {
			arg_name = plist_get(arg->names, j);
			check_list_of_vars_on_name(p, arg_name);

			plist_add(p->local_vars, new_plocal_var(arg_name, arg));
		}
	}

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
	match(name, ID);
	plist_add(os, name);
	return IP_GOTO;
}

// ### os explanation
//   _ - label to jmp to
enum IP_Code pser_local_inst_break(struct Pser *p, struct PList *os) {
	if (!p->loops->size)
		eet(pser_cur(p), NO_LOOP_TO_, USE_INSIDE_LOOP);
	consume(p);

	struct Loop *loop = p_last(p->loops);
	if (!loop->brek)
		loop->brek = take_label(LC_ELSE);

	plist_add(os, copy_str(loop->brek));
	return IP_BREAK;
}

// ### os explanation
//   _ - label to jmp to
enum IP_Code pser_local_inst_continue(struct Pser *p, struct PList *os) {
	if (!p->loops->size)
		eet(pser_cur(p), NO_LOOP_TO_, USE_INSIDE_LOOP);
	consume(p);

	struct Loop *loop = p_last(p->loops);
	if (!loop->cont)
		loop->cont = take_label(LC_ELSE);

	plist_add(os, copy_str(loop->cont));
	return IP_CONTINUE;
}

// ### os explanation
// ... - local instructions
//   _ - struct Loop *
enum IP_Code pser_local_inst_loop(struct Pser *p, struct PList *os) {
	consume(p); // skip вечно
	struct Loop *loop = new_loop(0, 0);
	plist_add(p->loops, loop);

	parse_block_of_local_inst(p, os);
	plist_add(os, loop);

	p->loops->size--; // remove loop from loops
	return IP_LOOP;
}

// ### os explanation
//   _ - return LocalExpr *
enum IP_Code pser_local_inst_return(struct Pser *p, struct PList *os) {
	consume(p); // skip воздать
	if (pser_cur(p)->code == COMMA) {
		plist_add(os, 0);
		consume(p); // skip ','
	} else
		plist_add(os, local_expression(p));
	return IP_RETURN;
}

// ### os explanation
//   _ - struct Token *name
//   _ - struct Type *
//   _ - struct LocalExpr *
enum IP_Code pser_local_inst_is(struct Pser *p, struct PList *os,
								struct Token *name) {
	plist_add(os, name);				// name
	consume(p);							// skip name
	consume(p);							// skip есть
	plist_add(os, type_expr(p));		// type
	match(pser_cur(p), EQU);			// skip '='
	plist_add(os, local_expression(p)); // value
	return IP_IS;
}

constr STR_BREAK = "обрыв";
constr STR_CONTINUE = "миновать";
constr STR_RETURN = "воздать";
constr STR_IS = "есть";

struct Inst *get_local_inst(struct Pser *p) {
	struct Token *c = pser_cur(p), *n;
	struct PList *os = new_plist(2);
	char *cv = vs(c);
	enum IP_Code code = IP_NONE;

	while (c->code == SLASHN || c->code == SEP)
		c = absorb(p);
	n = pser_next(p);

	// fill *os in funcs
	switch (c->code) {
	case EF:
		code = IP_EOI;
		break;
	case COMMA:
		absorb(p);
		code = IP_NONE;
		break;
	case ID:
		if (sc(cv, STR_ASM))
			code = inst_pser_asm(p, os);
		else if (sc(cv, STR_LET))
			code = pser_local_inst_let(p, os);
		else if (sc(cv, STR_GOTO))
			code = pser_local_inst_goto(p, os);
		else if (sc(cv, STR_BREAK))
			code = pser_local_inst_break(p, os);
		else if (sc(cv, STR_CONTINUE))
			code = pser_local_inst_continue(p, os);
		else if (sc(cv, STR_RETURN))
			code = pser_local_inst_return(p, os);
		else if (sc(cv, STR_LOOP))
			code = pser_local_inst_loop(p, os);

		else if (n->code == COLO)
			code = pser_local_inst_label(p, os, c);
		else if (sc(vs(n), STR_IS))
			code = pser_local_inst_is(p, os, c);

		if (code != IP_NONE)
			break;
	default:
		code = IP_LOCAL_EXPRESSION;
		plist_add(os, local_expression(p));
	}
	//
	// 	IP_IF_ELIF_ELSE,
	// 	IP_WHILE_LOOP,
	// 	IP_FOR_LOOP,
	//
	// 	IP_MATCH,

	return new_inst(p, code, os, c);
}
