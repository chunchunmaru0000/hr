#include "pser.h"
#include <stdint.h>
#include <stdio.h>

// TODO: is it possible to do better str search
enum IP_Code inst_pser_define(struct Pser *p) {
	struct Defn *d;
	char *view = (char *)absorb(p)->view;
	consume(p); // skip name token
	void *expr = expression(p);

	for (long i = 0; i < p->ds->size; i++) {
		d = plist_get(p->ds, i);

		if (sc(view, d->view)) {
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

	consume(p);
	expect(p, path, STR);

	plist_add(os, path);
	return IP_INCLUDE;
}

enum IP_Code inst_pser_asm(struct Pser *p, struct PList *os) {
	struct Token *code = absorb(p);

	consume(p); // skip str
	expect(p, code, STR);

	plist_add(os, code);
	return IP_ASM;
}

enum IP_Code inst_pser_enum(struct Pser *p, struct PList *os) {
	struct Token *enum_name = absorb(p), *cur, *num;
	plist_add(os, enum_name);

	cur = absorb(p);
	expect(p, cur, PAR_L);
	while (not_ef_and(PAR_R, cur)) {
		// consumes PAR_L or ID or num
		cur = absorb(p);
		if (cur->code == ID) {
			num = get_pser_token(p, 1);

			if (num->code == INT) {
				// so now its ID with number 1000iq tokens
				cur->number = num->number;
				cur->str = num->view; // and its view as str 2000iq tokens
				// REMEMBER: when ID and fpn = HAVE_NUM means have num
				cur->fpn = HAVE_NUM;
				consume(p); // consume cur
			} else
				cur->fpn = 0.0;

			plist_add(os, cur);
		} else if (cur->code != PAR_R)
			eet(p->f, cur, EXPECTED__ID, 0);
	}
	match(p, cur, PAR_R);

	return IP_DECLARE_ENUM;
}

const char *const TYPES_SIZES_NOT_MATCH =
	"Размеры типов для одного участка памяти должны быть одинаковы.";
const char *const NOT_FUN_SIGN_NEED_ARG_NAMES =
	"При объявлении функции полностью, а не только ее сигнатуры, все аргументы "
	"должны иметь имена.";

struct FunArg *new_arg() {
	struct FunArg *arg = malloc(sizeof(struct FunArg));
	arg->arg_names = new_plist(1);
	arg->type = 0;
	arg->either = 0;
	return arg;
}

struct FunArg *parse_arg(struct Pser *p, struct FunArg *from) {
	struct FunArg *arg = new_arg();
	struct Token *c = pser_cur(p);
	while (not_ef_and(COLO, c)) {
		expect(p, c, ID);
		plist_add(arg->arg_names, c);

		c = absorb(p);
		if (pser_cur(p)->code == DIV)
			c = absorb(p);
		else
			continue;
	}
	match(p, c, COLO);

	arg->type = type_expr(p);
	if (from && !types_sizes_do_match(from->type->code, arg->type->code))
		eet(p->f, c, TYPES_SIZES_NOT_MATCH, 0); // TODO: somehow get arg token

	c = pser_cur(p);
	if (c->code == COMMA) {
		consume(p);
		arg->either = parse_arg(p, arg);
	}

	return arg;
}

void parse_args(struct Pser *p, struct PList *os) {
	struct Token *c = absorb(p); // skip '('

	while (not_ef_and(PAR_R, c)) {
		plist_add(os, parse_arg(p, 0));
		c = pser_cur(p);
	}
	match(p, c, PAR_R);
}

enum IP_Code inst_pser_dare_fun(struct Pser *p, struct PList *os) {
	uc is_sign_flag = 0;
	uint32_t i;
	struct FunArg *arg;

	struct Token *cur = absorb(p); // skip фц
	expect(p, cur, ID);
	plist_add(os, cur);

	cur = absorb(p);
	if (cur->code == EXCL) {
		is_sign_flag = 1;
		cur = absorb(p); // skip !
	}
	expect(p, cur, PAR_L);

	parse_args(p, os);
	plist_add(os, type_expr(p));

	if (is_sign_flag) {
		// TODO: and need to add it to somewhere i believe
		return IP_DECLARE_FUNCTION_SIGNATURE;
	}

	for (i = 0; i < os->size - 1; i++) {
		arg = plist_get(os, i);

		if (arg->arg_names->size == 0)
			eet(p->f, cur, NOT_FUN_SIGN_NEED_ARG_NAMES, 0);
		// TODO: recursive check on either names also
	}
	plist_add(os, 0); // args terminator

	// parse block statement

	return IP_DECLARE_FUNCTION;
}
