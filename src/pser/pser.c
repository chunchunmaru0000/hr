#include "pser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
	p->same_name_funs = new_plist(16);

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

void free_inst(struct Inst *in) {
	plist_free(in->os);
	free(in);
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
	struct Inst *in;
	match(pser_cur(p), PAR_L);

	while (not_ef_and(PAR_R, pser_cur(p))) {
		in = get_local_inst(p);
		if (in->code == IP_NONE)
			free_inst(in);
		else
			plist_add(os, in);
	}

	match(pser_cur(p), PAR_R);
}

constr ARGS_NAMES_OVERLAP = "Аргумент с таким именем уже существует.";

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

constr ERR_WRONG_TOKEN = "Неверное выражение.";

#define ex_sg(n, b, c, e)                                                      \
	constr EXPECTED__##n = b c e;                                              \
	constr SUGGEST__##n = c;
ex_sg(STR, "Ожидалась ", "строка", ".");
ex_sg(INT, "Ожидалось ", "целое число", ".");
ex_sg(FPN, "Ожидалось ", "вещественное число", ".");
ex_sg(PAR_L, "Ожидалась '", "(", "' скобка.");
ex_sg(PAR_R, "Ожидалась '", ")", "' скобка.");
ex_sg(PAR_C_L, "Ожидалась '", "[", "' скобка.");
ex_sg(PAR_C_R, "Ожидалась '", "]", "' скобка.");
ex_sg(EQU, "Ожидался '", "=", "' знак равно.");
ex_sg(COLO, "Ожидалось '", ":", "'.");
ex_sg(ID, "Ожидалось ", "имя или слово", ".");
ex_sg(COMMA, "Ожидалась '", ",", "' запятая.");
ex_sg(SH_L, "Ожидалось '", "(#", "'.");
ex_sg(DOT, "Ожидалась '", ".", "' точка.");
ex_sg(CC, "Ожидалось '", "::", "'.");

constr STR_EOF = "_КОНЕЦ_ФАЙЛА_";
// words
constr STR_LET = "пусть";
constr STR_ASM = "_асм";
constr STR_GOTO = "идти";
constr STR_LOOP = "вечно";

constr STR_FUN = "фц";
constr STR_ENUM = "счет";
constr STR_STRUCT = "лик";
constr STR_AS = "окак";
constr STR_SIZE_OF = "мера";
constr STR_SIZE_OF_VAL = "размера";

struct Inst *get_global_inst(struct Pser *p) {
	struct Token *cur = pser_cur(p); //, *n;
	struct PList *os = new_plist(2);
	char *cv = vs(cur);
	enum IP_Code code = IP_NONE;

	while (cur->code == SLASHN || cur->code == SEP)
		cur = absorb(p);
	// n = pser_next(p);

	// fill *os in funcs
	switch (cur->code) {
	case EF:
		code = IP_EOI;
		break;
	case COMMA:
		absorb(p);
		code = IP_NONE;
		break;
	case ID:
		if (sc(cv, STR_FUN))
			code = inst_pser_dare_fun(p, os);
		else if (sc(cv, STR_LET))
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
	//	TODO
	//	global IP_DECLARE_LABEL, // like seem to be meaningless
	//	global IP_GOTO,

	return new_inst(p, code, os, cur);
}

struct PList *pse(struct Pser *p) {
	struct PList *is = new_plist(p->ts->cap_pace); // why not

	struct Inst *i = get_global_inst(p);
	while (i->code != IP_EOI) {
		if (i->code == IP_NONE) {
			free_inst(i);
		} else {
			if (i->code == IP_DECLARE_STRUCT)
				plist_add(parsed_structs, i);
			plist_add(is, i);
		}

		i = get_global_inst(p);
	}
	plist_add(is, i);

	return is;
}
