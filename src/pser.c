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
void pw(struct Fpfc *f, struct Token *t, const char *const msg,
		const char *const sgst) {
	if (!NEED_WARN)
		return;

	char *help;
	uint32_t token_chars_len, help_len, sgst_len = -1;
	if (sgst)
		sgst_len = strlen(sgst);

	token_chars_len =
		get_utf8_chars_to_pos((const char *)t->view->st, t->view->size);

	help_len = token_chars_len + 1 + sgst_len;
	help = malloc(help_len + 1);

	help[help_len] = 0; // terminate
	for (uint32_t i = 0; i < token_chars_len; i++)
		help[i] = UNDERLINE_CHAR; // fill with underline

	if (sgst) {
		help[token_chars_len] = '\n'; // split by \n
		memcpy(help + token_chars_len + 1, sgst, sgst_len);
	}

	fprintf(stderr, "%s%s:%d:%d%s ПРЕДУПРЕЖДЕНИЕ: %s%s\n", COLOR_WHITE, f->path,
			t->p->line, t->p->col, COLOR_LIGHT_PURPLE, msg, COLOR_RESET);
	print_source_line(f->code, t->p, COLOR_LIGHT_PURPLE, help);
}

void pwei_with_extra(struct ErrorInfo *info) {
	struct BList *sgst_list, *tmp_list;
	long sgst_len;

	if (info->extra_type == ET_NONE)
		pw(info->f, info->t, info->msg, info->sgst);
	else {
		sgst_len = strlen(info->sgst);
		sgst_list = new_blist(strlen(info->sgst));
		blat(sgst_list, (uc *)info->sgst, sgst_len);

		if (info->extra_type == ET_INT) {
			tmp_list = int_to_str((long)info->extra);
			blat_blist(sgst_list, tmp_list);
			blist_add(sgst_list, '\0');

			blist_clear_free(tmp_list);
		}

		pw(info->f, info->t, info->msg, (char *)sgst_list->st);
		blist_clear_free(sgst_list);
	}
}

void eei(struct Inst *in, const char *const msg, const char *const sgst) {
	eet(in->f, in->start_token, msg, sgst);
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

	p->errors = new_plist(2);
	p->warns = new_plist(2);

	p->enums = new_plist(8);
	p->structs = new_plist(8);

	p->global_vars = new_plist(16);
	p->local_vars = new_plist(16);

	return p;
}

void pser_err(struct Pser *p) {
	struct ErrorInfo *ei;
	int i;

	for (i = p->errors->size - 1; i >= 0; i--) {
		ei = plist_get(p->errors, i);
		etei(ei);
	}

	for (i = p->warns->size - 1; i >= 0; i--) {
		ei = plist_get(p->warns, i);
		pwei_with_extra(ei);
		free(ei);
	}
	plist_clear(p->warns);

	if (p->errors->size)
		exit(1);
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

void parse_block_of_local_inst(struct Pser *p, struct PList *os) {
	match(p, pser_cur(p), PAR_L);

	while (not_ef_and(PAR_R, pser_cur(p)))
		plist_add(os, get_local_inst(p));

	match(p, pser_cur(p), PAR_R);
}

const char *const ARGS_NAMES_OVERLAP =
	"Аргумент с таким именем уже существует.";

void check_list_of_vars_on_name(struct Pser *p, struct Token *name_to_check) {
	uint32_t i;
	struct PLocalVar *var;

	for (i = 0; i < p->local_vars->size; i++) {
		var = plist_get(p->local_vars, i);

		if (sc((char *)name_to_check->view->st, (char *)var->name->view->st))
			eet(p->f, name_to_check, ARGS_NAMES_OVERLAP,
				(char *)var->name->view->st);
	}
}
void check_list_of_args_on_name(struct Fpfc *f, struct PList *l,
								uint32_t from_arg, uint32_t from_name,
								struct Token *name_to_check) {
	uint32_t i, j;
	struct Arg *arg;
	struct Token *name;

	for (i = from_arg; i < l->size; i++) {
		arg = plist_get(l, i);

		for (j = i == from_arg ? from_name + 1 : 0; j < arg->names->size; j++) {
			name = plist_get(arg->names, j);

			if (sc((char *)name_to_check->view->st, (char *)name->view->st))
				eet(f, name, ARGS_NAMES_OVERLAP, (char *)name->view->st);
		}
	}
}
void check_list_of_args_on_uniq_names(struct Fpfc *f, struct PList *l,
									  uint32_t start_index) {
	uint32_t i, j;
	struct Arg *arg;
	struct Token *name;

	for (i = start_index; i < l->size; i++) {
		arg = plist_get(l, i);

		for (j = 0; j < arg->names->size; j++) {
			name = plist_get(arg->names, j);
			check_list_of_args_on_name(f, l, i, j, name);
		}
	}
}

struct PLocalVar *new_plocal_var(struct Token *name, struct Arg *arg) {
	struct PLocalVar *var = malloc(sizeof(struct PLocalVar));
	var->name = name;
	var->type = arg->type;
	var->var_size = arg->arg_size;
	return var;
}

const char *const ERR_WRONG_TOKEN = "Неверное выражение.";

const char *const EXPECTED__STR = "Ожидалась строка.";
const char *const EXPECTED__INT = "Ожидалось целое число.";
const char *const EXPECTED__FPN = "Ожидалось вещественное число.";
const char *const EXPECTED__PAR_L = "Ожидалась '(' скобка.";
const char *const EXPECTED__PAR_R = "Ожидалась ')' скобка.";
const char *const EXPECTED__PAR_C_L = "Ожидалась '[' скобка.";
const char *const EXPECTED__PAR_C_R = "Ожидалась ']' скобка.";
const char *const EXPECTED__EQU = "Ожидался '=' знак равно.";
const char *const EXPECTED__COLO = "Ожидалось ':'.";
const char *const EXPECTED__ID = "Ожидалось имя или слово.";

const char *const SUGGEST__STR = "строка";
const char *const SUGGEST__PAR_L = "(";
const char *const SUGGEST__PAR_R = ")";
const char *const SUGGEST__PAR_C_L = "[";
const char *const SUGGEST__PAR_C_R = "]";
const char *const SUGGEST__EQU = "=";
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
const char *const STR_GOTO = "идти";
const char *const STR_LOOP = "вечно";

const char *const STR_FUN = "фц";
const char *const STR_ENUM = "счет";
const char *const STR_STRUCT = "лик";
const char *const STR_AS = "окак";

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
		} else if (sc(cv, STR_LET))
			code = inst_pser_global_let(p, os);
		else if (sc(cv, STR_ASM))
			code = inst_pser_asm(p, os);
		else if (sc(cv, STR_ENUM))
			code = inst_pser_enum(p, os);
		else if (sc(cv, STR_STRUCT))
			code = inst_pser_struct(p, os);

		if (code != IP_NONE)
			break;
	default:
		eet(p->f, cur, ERR_WRONG_TOKEN, 0);
	}
	//	TODO
	//	global expression,
	//
	//	IP_DECLARE_FUNCTION body,
	//
	//	IP_DEFINE
	//	IP_INCLUDE
	//	global IP_DECLARE_LABEL, // like seem to be meaningless
	//	global IP_GOTO,

	return new_inst(p, code, os, cur);
}

void include_in_is(struct Pser *p, struct PList *is, struct Inst *i) {
	struct Token *path = plist_get(i->os, 0);
	blist_add(path->str, 0); // string 0 terminator

	struct Pser *tmp_p = new_pser((char *)path->str->st, p->debug);
	plist_free(tmp_p->enums);
	tmp_p->enums = p->enums;
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
