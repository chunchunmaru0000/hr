#include "pser.h"

constr TYPES_SIZES_NOT_MATCH =
	"Размеры типов для одного участка памяти должны быть одинаковы.";
constr SEVERAL_ARGS_CANT_SHARE_MEM =
	"Несколько аргументов объявленных таким образом не могут иметь синонимы с "
	"другими типом, так как они принадлежат к разным участкам памяти.";
constr SUGGEST_DELETE_ARGS_OR_COMMA = "удалить аргументы или запятую";
constr COMMA_ARGS_CAN_BE_ONLY_BY_ONE =
	"Аргументы для одного участка памяти могут быть только по одному, иначе "
	"это уже не один участок памяти.";
constr SUGGEST_CHANGE_ARG_TYPE_SIZE = "изменить размер типа";
constr EXPECTED_COLO_OR_DIV_IN_ARG =
	"В данном месте аргумента ожидалось ':' или '/'.";

struct Arg *new_arg() {
	struct Arg *arg = malloc(sizeof(struct Arg));
	arg->names = new_plist(1);
	arg->type = 0;
	arg->offset = 0;
	arg->arg_size = 0;
	return arg;
}

// this is kinda one item stack to know eithers is_one_mem or not
// cuz do it via function atgument is kinda shit so at least global var
uc is_one_memories_flag;

struct PList *parse_arg(struct Pser *p, struct Arg *from, long args_offset) {
	struct PList *args = new_plist(2), *eithers;
	struct Arg *arg = new_arg();
	struct TypeExpr *type;
	uint32_t i;
	int type_size, colo_pos;
	plist_add(args, arg);

	struct Token *c = pser_cur(p);
	expect(c, ID); // ensures min one name
	plist_add(arg->names, c);

	while (not_ef_and(COLO, c)) {
		c = absorb(p);
		if (c->code == ID) {
			arg = new_arg();
			plist_add(arg->names, c);
			plist_add(args, arg);
			continue;
		} else if (c->code == DIV) {
			while (c->code == DIV) {
				c = absorb(p);
				expect(c, ID);
				plist_add(arg->names, c);
				c = pser_cur(p);
			}
			continue;
		}
		if (c->code != COLO)
			eet(c, EXPECTED_COLO_OR_DIV_IN_ARG, 0);
		break;
	}
	colo_pos = p->pos;
	match(c, COLO);
	uc is_one_memory = args->size == 1;
	is_one_memories_flag = is_one_memory;

	type = type_expr(p);
	type_size = size_of_type(p, type);

	if (is_one_memory && from && (type_size != size_of_type(p, from->type)))
		eet(pser_prev(p), TYPES_SIZES_NOT_MATCH, SUGGEST_CHANGE_ARG_TYPE_SIZE);
	if (!is_one_memory && from)
		eet(get_pser_token((p), colo_pos - p->pos - 1),
			SEVERAL_ARGS_CANT_SHARE_MEM, SUGGEST_DELETE_ARGS_OR_COMMA);

	c = pser_cur(p);
	if (c->code == COMMA) {
		if (!is_one_memory)
			eet(c, SEVERAL_ARGS_CANT_SHARE_MEM, SUGGEST_DELETE_ARGS_OR_COMMA);
		consume(p); // consume ,

		// set thing to the single arg
		arg->offset = args_offset;
		arg->type = type;
		arg->arg_size = type_size;

		// get new arg
		eithers = parse_arg(p, arg, args_offset);
		if (!is_one_memories_flag && eithers->size != 1)
			eet(c, COMMA_ARGS_CAN_BE_ONLY_BY_ONE, SUGGEST_DELETE_ARGS_OR_COMMA);
		// add all and free
		for (i = 0; i < eithers->size; i++)
			plist_add(args, plist_get(eithers, i));
		plist_free(eithers);
	} else {
		for (i = 0; i < args->size; i++) {
			arg = plist_get(args, i);
			arg->offset = args_offset + i * type_size;
			// here multiple args can have one type that is shared memory
			arg->type = type;
			arg->arg_size = type_size;
		}
	}

	return args;
}

void parse_args(struct Pser *p, struct PList *os) {
	long args_offset = 0;
	struct Token *c = absorb(p); // skip '('
	struct PList *args;
	struct Arg *arg;
	uint32_t i;

	while (not_ef_and(PAR_R, c)) {
		args = parse_arg(p, 0, args_offset);

		for (i = 0; i < args->size; i++) {
			arg = plist_get(args, i);
			plist_add(os, arg);
		}
		args_offset = arg->offset + arg->arg_size;

		plist_free(args);
		c = pser_cur(p);
	}
	match(c, PAR_R);
}
