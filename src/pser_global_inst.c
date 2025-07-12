#include "pser.h"
#include <stdint.h>

// TODO: is it possible to do better str search
enum IP_Code inst_pser_define(struct Pser *p) {
	struct Defn *d;
	struct BList *view = absorb(p)->view;
	consume(p); // skip name token
	void *expr = expression(p);

	for (long i = 0; i < p->ds->size; i++) {
		d = plist_get(p->ds, i);

		if (sc((char *)view->st, (char *)d->view->st)) {
			// expression can be freed here cuz it doesnt holds pointers that
			// it mallocs, it only borrows them
			free(d->value);
			d->value = expr;
			return IP_NONE;
		}
	}

	d = malloc(sizeof(struct Defn));
	d->view = view;
	d->value = expr;
	plist_add(p->ds, d);
	return IP_NONE;
}

enum IP_Code inst_pser_include(struct Pser *p, struct PList *os) {
	// TODO: make relative folder addressation
	struct Token *path = absorb(p);

	match(p, path, STR);

	plist_add(os, path);
	return IP_INCLUDE;
}

enum IP_Code inst_pser_asm(struct Pser *p, struct PList *os) {
	struct Token *code = absorb(p);

	match(p, code, STR);

	plist_add(os, code);
	return IP_ASM;
}

enum IP_Code inst_pser_enum(struct Pser *p, struct PList *os) {
	struct Token *enum_name = absorb(p);
	expect(p, enum_name, ID);
	plist_add(os, enum_name);

	struct Token *c, *num;
	struct Defn *defn;
	long counter = 0;

	c = absorb(p);
	expect(p, c, PAR_L);

	for (c = absorb(p); not_ef_and(PAR_R, c); c = absorb(p)) {
		expect(p, c, ID);
		defn = malloc(sizeof(struct Defn));
		defn->view = new_blist(0);
		blat_blist(defn->view, enum_name->view); // enum name
		blist_add(defn->view, '.');				 // .
		blat_blist(defn->view, c->view);		 // thing name

		blist_add(defn->view, '\0'); // \0 terminated
		defn->view->size--;

		num = get_pser_token(p, 1);

		if (num->code == INT) {
			defn->value = (void *)num->number;
			consume(p); // consume ID
		} else
			defn->value = (void *)counter;

		plist_add(p->ds, defn);
		plist_add(os, defn);
		counter++;
	}
	match(p, c, PAR_R);

	return IP_DECLARE_ENUM;
}

const char *const TYPES_SIZES_NOT_MATCH =
	"Размеры типов для одного участка памяти должны быть одинаковы.";
const char *const FUN_SIGNATURES_OVERLAP =
	"Сигнатура данной функции повторяет сигнатуру другой уже объявленной "
	"функции, даже если типы записаны по разному но равны по смыслу(например "
	"'*ц8' и 'стр'), их сигнатуры будут равны.";
const char *const SUGGEST_FIX_FUN_SIGNATURES_OVERLAP =
	"изменить типы аругментов функции";
const char *const SEVERAL_ARGS_CANT_SHARE_MEM =
	"Несколько аргументов объявленных таким образом не могут иметь синонимы с "
	"другими типом, так как они принадлежат к разным участкам памяти.";
const char *const DELETE_ARGS_OR_COMMA = "удалить аргументы или запятую";
const char *const COMMA_ARGS_CAN_BE_ONLY_BY_ONE =
	"Аргументы для одного участка памяти могут быть только по одному, иначе "
	"это уже не один участок памяти.";

struct FunArg *new_arg() {
	struct FunArg *arg = malloc(sizeof(struct FunArg));
	arg->arg_names = new_plist(1);
	arg->type = 0;
	arg->either = 0;
	return arg;
}

struct PList *parse_arg(struct Pser *p, struct FunArg *from) {
	struct PList *args = new_plist(2), *eithers;
	struct FunArg *arg = new_arg();
	uint32_t i;
	struct TypeExpr *type;
	plist_add(args, arg);

	struct Token *c = pser_cur(p);
	expect(p, c, ID); // ensures min one name
	plist_add(arg->arg_names, c);

	while (not_ef_and(COLO, c)) {
		c = absorb(p);
		if (c->code == ID) {
			arg = new_arg();
			plist_add(arg->arg_names, c);
			plist_add(args, arg);
		} else
			while (c->code == DIV) {
				c = absorb(p);
				expect(p, c, ID);
				plist_add(arg->arg_names, c);
				c = pser_cur(p);
			}
	}
	match(p, c, COLO);
	uc is_one_memory = args->size == 1;

	type = type_expr(p);

	if (is_one_memory && from &&
		!types_sizes_do_match(from->type->code, type->code))
		eet(p->f, c, TYPES_SIZES_NOT_MATCH,
			0); // TODO: somehow get arg type token

	c = pser_cur(p);
	if (c->code == COMMA) {
		if (!is_one_memory)
			eet(p->f, c, SEVERAL_ARGS_CANT_SHARE_MEM, DELETE_ARGS_OR_COMMA);
		consume(p);

		arg->type = type;
		eithers = parse_arg(p, arg);
		if (eithers->size == 1)
			arg->either = plist_get(eithers, 0);
		else
			eet(p->f, c, COMMA_ARGS_CAN_BE_ONLY_BY_ONE, 0);
	} else {
		for (uint32_t i = 0; i < args->size; i++) {
			arg = plist_get(args, i);
			arg->type = type;
		}
	}

	return args;
}

void parse_args(struct Pser *p, struct PList *os) {
	struct Token *c = absorb(p); // skip '('
	struct PList *args;
	uint32_t i;

	while (not_ef_and(PAR_R, c)) {
		// TODO: plat
		args = parse_arg(p, 0);
		for (i = 0; i < args->size; i++)
			plist_add(os, plist_get(args, i));

		plist_free(args);
		c = pser_cur(p);
	}
	match(p, c, PAR_R);
}

enum IP_Code inst_pser_dare_fun(struct Pser *p, struct PList *os) {
	uint32_t i;

	struct FunArg *arg;
	struct TypeExpr *type;
	struct GlobVar *fun_variable = malloc(sizeof(struct GlobVar)), *tmp_var;
	struct TypeExpr *fun_type = get_type_expr(TC_FUN);
	fun_type->data.args_types = new_plist(2);

	struct Token *cur = absorb(p); // skip фц
	expect(p, cur, ID);
	plist_add(os, cur); // fun name
	fun_variable->name = cur;
	fun_variable->type = fun_type;

	cur = absorb(p);
	expect(p, cur, PAR_L);

	parse_args(p, os);
	for (i = 1; i < os->size; i++) {
		arg = plist_get(os, i);

		// it haves here types cuz fun type args are types
		// its primary for signature
		plist_add(fun_type->data.args_types, arg->type);
	}

	type = type_expr(p);
	plist_add(os, type);
	plist_add(fun_type->data.args_types, type);

	for (i = 0; i < p->global_vars->size; i++) {
		tmp_var = plist_get(p->global_vars, i);

		if (sc((char *)tmp_var->name->view->st,
			   (char *)fun_variable->name->view->st) &&
			are_types_equal(tmp_var->type, fun_variable->type))
			eet(p->f, fun_variable->name, FUN_SIGNATURES_OVERLAP,
				SUGGEST_FIX_FUN_SIGNATURES_OVERLAP);
	}

	get_global_signature(fun_variable);
	plist_add(p->global_vars, fun_variable);
	plist_set(os, 0, fun_variable);

	plist_add(os, 0); // args terminator
	match(p, pser_cur(p), PAR_L);

	// parse block statement

	// * локальные инструкции хранятся в операндах глобальной инструкции,
	//   после терминирующего нуля

	match(p, pser_cur(p), PAR_R);

	return IP_DECLARE_FUNCTION;
}
