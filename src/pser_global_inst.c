#include "pser.h"
#include <stdint.h>

// ### os explanation:
//   _ - assembly string token
enum IP_Code inst_pser_asm(struct Pser *p, struct PList *os) {
	struct Token *code = absorb(p);

	match(p, code, STR);

	plist_add(os, code);
	return IP_ASM;
}

// ### os explanation:
//   _ - name
// ... - defns where name is name and value is num
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

		convert_blist_to_blist_from_str(defn->view);

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
const char *const TOO_MUCH_ARGS_FOR_NOW =
	"Слишком много аргументов фукнции, на данный момент максимальное "
	"количество аргуентов функции: 7.";
const char *const SUGGEST_CUT_ARGS_SIZE = "уменьшить количество аргументов";

struct Arg *new_arg() {
	struct Arg *arg = malloc(sizeof(struct Arg));
	arg->names = new_plist(1);
	arg->type = 0;
	arg->offset = 0;
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
	int type_size;
	plist_add(args, arg);

	struct Token *c = pser_cur(p);
	expect(p, c, ID); // ensures min one name
	plist_add(arg->names, c);

	while (not_ef_and(COLO, c)) {
		c = absorb(p);
		if (c->code == ID) {
			arg = new_arg();
			plist_add(arg->names, c);
			plist_add(args, arg);
		} else
			while (c->code == DIV) {
				c = absorb(p);
				expect(p, c, ID);
				plist_add(arg->names, c);
				c = pser_cur(p);
			}
	}
	match(p, c, COLO);
	uc is_one_memory = args->size == 1;
	is_one_memories_flag = is_one_memory;

	type = type_expr(p);
	type_size = size_of_type(type);

	// TODO: somehow get arg type token
	if (is_one_memory && from && (type_size != size_of_type(from->type)))
		eet(p->f, c, TYPES_SIZES_NOT_MATCH, 0);
	if (!is_one_memory && from)
		eet(p->f, c, SEVERAL_ARGS_CANT_SHARE_MEM, DELETE_ARGS_OR_COMMA);

	c = pser_cur(p);
	if (c->code == COMMA) {
		if (!is_one_memory)
			eet(p->f, c, SEVERAL_ARGS_CANT_SHARE_MEM, DELETE_ARGS_OR_COMMA);
		consume(p); // consume ,

		// set thing to the single arg
		arg->offset = args_offset;
		arg->type = type;

		// get new arg
		eithers = parse_arg(p, arg, args_offset);
		if (!is_one_memories_flag && eithers->size != 1)
			eet(p->f, c, COMMA_ARGS_CAN_BE_ONLY_BY_ONE, DELETE_ARGS_OR_COMMA);
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
		args_offset = arg->offset + size_of_type(arg->type);

		plist_free(args);
		c = pser_cur(p);
	}
	match(p, c, PAR_R);
}

// ### os explanation:
//   _ - name
// ... - fields that are Arg's
enum IP_Code inst_pser_struct(struct Pser *p, struct PList *os) {
	struct Token *c = absorb(p); // skip лик
	expect(p, c, ID);
	plist_add(os, c); // struct name

	expect(p, absorb(p), PAR_L);
	parse_args(p, os);

	return IP_DECLARE_STRUCT;
}

// ### os explanation:
//   _ - GlobVar with name and type
// ... - Arg's
//   0 - zero terminator
// ... - local inctrustions
enum IP_Code inst_pser_dare_fun(struct Pser *p, struct PList *os) {
	uint32_t i;

	struct Arg *arg;
	struct TypeExpr *type;
	struct GlobVar *fun_variable = malloc(sizeof(struct GlobVar)), *tmp_var;
	struct TypeExpr *fun_type = get_type_expr(TC_FUN);
	fun_type->data.args_types = new_plist(2);

	struct Token *cur = absorb(p); // skip фц
	expect(p, cur, ID);
	plist_add(os, 0); // reserved place for variable
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

	if (fun_type->data.args_types->size > MAX_ARGS_ON_REGISTERS)
		eet(p->f, fun_variable->name, TOO_MUCH_ARGS_FOR_NOW,
			SUGGEST_CUT_ARGS_SIZE);

	type = type_expr(p);
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

	parse_block_of_local_inst(p, os);

	return IP_DECLARE_FUNCTION;
}
