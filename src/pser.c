#include "pser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void ee_token(struct Fpfc *f, struct Token *t, char *msg) { // error exit
	fprintf(stderr, "%s%s:%d:%d:%s ОШИБКА: %s [%s]:[%d]%s\n", COLOR_WHITE,
			f->path, t->p->line, t->p->col, COLOR_RED, msg, t->view->st,
			t->code, COLOR_RESET);
	print_source_line(f->code, t->p, COLOR_LIGHT_RED, 0);
	exit(1);
}

// print warning
void pw(struct Fpfc *f, struct Pos *p, const char *const msg) {
	if (NEED_WARN) {
		fprintf(stderr, "%s%s:%d:%d%s ПРЕДУПРЕЖДЕНИЕ: %s%s\n", COLOR_WHITE,
				f->path, p->line, p->col, COLOR_LIGHT_PURPLE, msg, COLOR_RESET);
		print_source_line(f->code, p, COLOR_LIGHT_PURPLE, 0);
	}
}

void eei(struct Fpfc *f, struct Inst *in, const char *const msg,
		 const char *const sgst) {
	eet(f, in->start_token, msg, sgst);
}

struct Pser *new_pser(char *filename, uc debug) {
	struct Pser *p = malloc(sizeof(struct Pser));
	struct Tzer *t = new_tzer(filename);
	p->f = t->f;

	struct PList *ts = tze(t, 10);
	free(t);
	p->pos = 0;
	p->ts = ts;
	p->debug = debug;
	p->ds = new_plist(8);
	p->global_vars = new_plist(16);
	p->structs = new_plist(8);
	return p;
}

struct Inst *new_inst(struct Pser *p, enum IP_Code code, struct PList *os,
					  struct Token *t) {
	struct Inst *i = malloc(sizeof(struct Inst));
	i->f = p->f;
	i->start_token = t;

	i->os = os;
	i->code = code;
	return i;
}

struct Token *get_pser_token(struct Pser *p, long off) {
	long i = p->pos + off;
	return p->ts->size > i ? p->ts->st[i] : p->ts->st[p->ts->size - 1];
}

struct Token *next_pser_get(struct Pser *p, long off) {
	consume(p);
	return pser_cur(p);
	// p->pos++;
	// return get_pser_token(p, off);
}

// TODO:
struct Defn *is_defn(struct Pser *p, char *v) {
	struct Defn *d;
	for (uint32_t i = 0; i < p->ds->size; i++) {
		d = plist_get(p->ds, i);
		if (sc(v, (char *)d->view->st))
			return d;
	}
	return 0;
}

void parse_block_of_local_inst(struct Pser *p, struct PList *os) {
	match(p, pser_cur(p), PAR_L);

	while (not_ef_and(PAR_R, pser_cur(p)))
		plist_add(os, get_local_inst(p));

	match(p, pser_cur(p), PAR_R);
}

const char *const ERR_WRONG_TOKEN = "Неверное выражение.";

const char *const EXPECTED__STR = "Ожидалась строка.";
const char *const EXPECTED__INT = "Ожидалось целое число.";
const char *const EXPECTED__FPN = "Ожидалось вещественное число.";
const char *const EXPECTED__PAR_L = "Ожидалась '(' скобка.";
const char *const EXPECTED__PAR_R = "Ожидалась ')' скобка.";
const char *const EXPECTED__PAR_C_L = "Ожидалась '[' скобка.";
const char *const EXPECTED__PAR_C_R = "Ожидалась ']' скобка.";
const char *const EXPECTED__COLO = "Ожидалось ':'.";
const char *const EXPECTED__ID = "Ожидалось имя или слово.";

const char *const SUGGEST__STR = "строка";
const char *const SUGGEST__PAR_L = "(";
const char *const SUGGEST__PAR_R = ")";
const char *const SUGGEST__PAR_C_L = "[";
const char *const SUGGEST__PAR_C_R = "]";
const char *const SUGGEST__COLO = ":";
const char *const SUGGEST__ID = "имя";
const char *const SUGGEST__INT = "целое";
const char *const SUGGEST__FPN = "вещественное";

const char *const STR_EOF = "_КОНЕЦ_ФАЙЛА_";
// parser directives
const char *const STR_DEFINE = "вот";
const char *const STR_INCLUDE = "влечь";
// words
const char *const STR_LET = "пусть";
const char *const STR_ASM = "_асм";

const char *const STR_FUN = "фц";
const char *const STR_ENUM = "счет";
const char *const STR_STRUCT = "лик";

struct Inst *get_global_inst(struct Pser *p) {
	struct Token *cur = pser_cur(p), *n;
	struct PList *os = new_plist(2);
	char *cv = (char *)cur->view->st;
	enum IP_Code code = IP_NONE;

	while (cur->code == SLASHN || cur->code == SEP)
		cur = absorb(p);
	n = get_pser_token(p, 1);

	// fill *os in funcs
	switch (cur->code) {
	case EF:
		code = IP_EOI;
		break;
	case ID:
		if (sc(cv, STR_FUN)) {
			os->cap_pace = 16;
			code = inst_pser_dare_fun(p, os);
		} else if (sc(cv, STR_ASM))
			code = inst_pser_asm(p, os);
		else if (sc(cv, STR_INCLUDE))
			code = inst_pser_include(p, os);
		else if (sc(cv, STR_DEFINE))
			code = inst_pser_define(p);
		else if (sc(cv, STR_ENUM))
			code = inst_pser_enum(p, os);
		else if (sc(cv, STR_STRUCT))
			code = inst_pser_struct(p, os);

		if (code != IP_NONE)
			break;
	default:
		eet(p->f, cur, ERR_WRONG_TOKEN, 0);
	}
	// 	IP_DECLARE_FUNCTION body,
	//
	// 	global IP_LET,
	//
	// 	global IP_DECLARE_LABEL,
	// 	global IP_GOTO,
	//
	// 	global expression,

	return new_inst(p, code, os, cur);
}

void include_in_is(struct Pser *p, struct PList *is, struct Inst *i) {
	struct Token *path = plist_get(i->os, 0);
	blist_add(path->str, 0); // string 0 terminator

	struct Pser *tmp_p = new_pser((char *)path->str->st, p->debug);
	plist_free(tmp_p->ds);
	tmp_p->ds = p->ds;
	struct PList *inc = pse(tmp_p);

	for (uint32_t j = 0; j < inc->size; j++)
		plist_add(is, plist_get(inc, j));

	free(tmp_p);
	free(inc);
}

struct PList *pse(struct Pser *p) {
	struct PList *is = new_plist(p->ts->cap_pace); // why not

	struct Inst *i = get_global_inst(p);
	while (i->code != IP_EOI) {
		if (i->code == IP_INCLUDE)
			include_in_is(p, is, i);
		else if (i->code != IP_NONE) {
			if (i->code == IP_DECLARE_STRUCT)
				plist_add(p->structs, i);
			plist_add(is, i);
		}
		i = get_global_inst(p);
	}
	plist_add(is, i);

	return is;
}
