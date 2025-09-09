#include "pser.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

struct Pser *new_pser(struct Fpfc *f, struct PList *tokens, char *filename,
					  uc debug) {
	struct Pser *p = malloc(sizeof(struct Pser));
	p->f = f;
	p->pos = 0;
	p->ts = tokens;
	p->debug = debug;

	p->errors = new_plist(2);
	p->warns = new_plist(2);

	p->enums = new_plist(8);

	if (parsed_structs == 0)
		parsed_structs = new_plist(8);

	p->global_vars = new_plist(16);
	p->local_vars = new_plist(16);

	return p;
}
struct PList *parsed_structs = 0;

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
	match(pser_cur(p), PAR_L);

	while (not_ef_and(PAR_R, pser_cur(p)))
		plist_add(os, get_local_inst(p));

	match(pser_cur(p), PAR_R);
}

const char *const ARGS_NAMES_OVERLAP =
	"Аргумент с таким именем уже существует.";

void check_list_of_vars_on_name(struct Pser *p, struct Token *name_to_check) {
	uint32_t i;
	struct PLocalVar *var;

	for (i = 0; i < p->local_vars->size; i++) {
		var = plist_get(p->local_vars, i);

		if (vc(name_to_check, var->name))
			eet(name_to_check, ARGS_NAMES_OVERLAP, vs(var->name));
	}
}
void check_list_of_args_on_name(struct PList *l, uint32_t from_arg,
								uint32_t from_name,
								struct Token *name_to_check) {
	uint32_t i, j;
	struct Arg *arg;
	struct Token *name;

	for (i = from_arg; i < l->size; i++) {
		arg = plist_get(l, i);

		for (j = i == from_arg ? from_name + 1 : 0; j < arg->names->size; j++) {
			name = plist_get(arg->names, j);

			if (vc(name_to_check, name))
				eet(name, ARGS_NAMES_OVERLAP, vs(name));
		}
	}
}
void check_list_of_args_on_uniq_names(struct PList *l, uint32_t start_index) {
	uint32_t i, j;
	struct Arg *arg;
	struct Token *name;

	for (i = start_index; i < l->size; i++) {
		arg = plist_get(l, i);

		for (j = 0; j < arg->names->size; j++) {
			name = plist_get(arg->names, j);
			check_list_of_args_on_name(l, i, j, name);
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

// struct GlobExpr *copy_glob(struct GlobExpr *e) {
// 	struct GlobExpr *copy = malloc(sizeof(struct GlobExpr));
//
// 	copy->from = e->from;
// 	copy->code = e->code;
// 	copy->type = e->type; // noy sure but
// 	copy->globs = e->globs ? copy_globs(e->globs) : 0;
//
// 	copy->tvar = malloc(sizeof(struct Token));
// 	copy_token(copy->tvar, e->tvar);
// 	if (e->code == CT_STR || e->code == CT_STR_PTR) {
// 		copy->tvar->view = copy_str(e->tvar->view);
// 		copy->tvar->str = copy_str(e->tvar->str);
// 	}
//
// 	return copy;
// }
//
// struct PList *copy_globs(struct PList *globs) {
// 	uint32_t i;
// 	struct PList *gs = new_plist(globs->size);
//
// 	for (i = 0; i < globs->size; i++)
// 		plist_set(gs, i, copy_glob(plist_get(globs, i)));
//
// 	return gs;
// }

struct PList *find_lik_os(struct BList *name) {
	struct Inst *in;
	struct Token *s_name;
	uint32_t i;

	for (i = 0; i < parsed_structs->size; i++) {
		in = plist_get(parsed_structs, i);
		s_name = plist_get(in->os, DCLR_STRUCT_NAME);

		if (sc((char *)name->st, vs(s_name)))
			return in->os;
	}

	return 0;
}

struct Arg *get_arg_of_next_offset(struct PList *lik_os, long last_offset) {
	struct Arg *arg;
	uint32_t i;

	for (i = DCLR_STRUCT_ARGS; i < lik_os->size; i++) {
		arg = plist_get(lik_os, i);

		if (arg->offset > last_offset)
			return arg;
	}
	exit(226);
}

struct Arg *get_arg_by_mem_index(struct PList *lik_os, uint32_t mem_index) {
	struct Arg *arg;
	long last_offset = -1;
	uint32_t i;

	for (i = DCLR_STRUCT_ARGS; i < lik_os->size; i++) {
		arg = plist_get(lik_os, i);

		if (last_offset != arg->offset) {
			if (mem_index == 0)
				return arg;
			mem_index--;
		}

		last_offset = arg->offset;
	}
	exit(225);
}

void free_glob_expr(struct GlobExpr *e) {
	enum CT_Code c = e->code;
	uint32_t i;

	if (c == CT_INT || c == CT_REAL || c == CT_ZERO || c == CT_FUN ||
		c == CT_GLOBAL || c == CT_GLOBAL_PTR) {
	just_free_token_and_e:
		free(e->tvar);
		free(e);
	} else if (c == CT_ARR || c == CT_ARR_PTR || c == CT_STRUCT ||
			   c == CT_STRUCT_PTR) {
		if (e->from)
			goto just_free_token_and_e;

		if (!e->globs)
			exit(222);

		for (i = 0; i < e->globs->size; i++)
			free_glob_expr(plist_get(e->globs, i));
		plist_free(e->globs);
		// HERE TOKEN IS NOT COPIED
		free(e);
	} else if (c == CT_STR || c == CT_STR_PTR) {
		blist_clear_free(e->tvar->view);
		blist_clear_free(e->tvar->str);
		goto just_free_token_and_e;
	} else
		exit(221);
}

long unsafe_size_of_global_value(struct GlobExpr *e) {
	if (e->type)
		return unsafe_size_of_type(e->type);
	enum CT_Code code = e->code;

	long res = 0;
	if (code == CT_INT)
		res = QWORD;
	else if (code == CT_REAL)
		res = QWORD;
	else if (code == CT_FUN)
		res = QWORD;
	else if (code == CT_STR)
		res = e->tvar->str->size + 1; // +1 for \0
	else if (code == CT_ARR)
		exit(217);
	else if (code == CT_STRUCT)
		exit(218);
	else if (code == CT_STR_PTR)
		res = QWORD;
	else if (code == CT_ARR_PTR)
		res = QWORD;
	else if (code == CT_STRUCT_PTR)
		res = QWORD;
	else if (code == CT_GLOBAL)
		exit(219);
	else if (code == CT_GLOBAL_PTR)
		res = QWORD;
	else if (code == CT_ZERO)
		exit(216);

	return res;
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
const char *const EXPECTED__COMMA = "Ожидалась ',' запятая.";
const char *const EXPECTED__SH_L = "Ожидалось '(#'.";

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
const char *const SUGGEST__COMMA = ",";
const char *const SUGGEST__SH_L = "(#";

const char *const STR_EOF = "_КОНЕЦ_ФАЙЛА_";
// words
const char *const STR_LET = "пусть";
const char *const STR_ASM = "_асм";
const char *const STR_GOTO = "идти";
const char *const STR_LOOP = "вечно";

const char *const STR_FUN = "фц";
const char *const STR_ENUM = "счет";
const char *const STR_STRUCT = "лик";
const char *const STR_AS = "окак";
const char *const STR_SIZE_OF = "мера";
const char *const STR_SIZE_OF_VAL = "размера";

struct Inst *get_global_inst(struct Pser *p) {
	struct Token *cur = pser_cur(p), *n;
	struct PList *os = new_plist(2);
	char *cv = vs(cur);
	enum IP_Code code = IP_NONE;

	while (cur->code == SLASHN || cur->code == SEP)
		cur = absorb(p);
	n = pser_next(p);

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
		eet(cur, ERR_WRONG_TOKEN, vs(cur));
	}
	//	TODO global expression TYPES,
	//	TODO
	//
	//	IP_DECLARE_FUNCTION body,
	//
	//	global IP_DECLARE_LABEL, // like seem to be meaningless
	//	global IP_GOTO,

	return new_inst(p, code, os, cur);
}

struct PList *pse(struct Pser *p) {
	struct PList *is = new_plist(p->ts->cap_pace); // why not

	struct Inst *i = get_global_inst(p);
	while (i->code != IP_EOI) {
		if (i->code != IP_NONE) {
			if (i->code == IP_DECLARE_STRUCT)
				plist_add(parsed_structs, i);
			plist_add(is, i);
		}
		i = get_global_inst(p);
	}
	plist_add(is, i);

	return is;
}
