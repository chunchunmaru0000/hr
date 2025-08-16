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

const char *const ENUM_ITEM_NAME_OVERLAP =
	"Значение счета с таким же именем уже существует.";
const char *const EXPECTED_INT_GLOB_EXPR =
	"Результатом глобального выражения для значения счета может быть только "
	"целое число.";

// ### os explanation:
//   _ - name
// ... - defns where name is name and value is num
enum IP_Code inst_pser_enum(struct Pser *p, struct PList *os) {
	struct Token *enum_name = absorb(p);
	expect(p, enum_name, ID);
	plist_add(os, enum_name);

	struct Token *c;
	struct GlobExpr *e;
	struct Defn *defn, *tmp_defn;
	long counter = 0;
	uint32_t i;

	c = absorb(p);
	expect(p, c, PAR_L);

	for (c = absorb(p); not_ef_and(PAR_R, c);) {
		expect(p, c, ID);
		defn = malloc(sizeof(struct Defn));
		defn->view = new_blist(0);
		blat_blist(defn->view, enum_name->view); // enum name
		blist_add(defn->view, '.');				 // .
		blat_blist(defn->view, c->view);		 // thing name

		convert_blist_to_blist_from_str(defn->view);

		// check here for identical names in enums items
		for (i = 0; i < p->enums->size; i++) {
			tmp_defn = plist_get(p->enums, i);

			if (sc((char *)defn->view->st, (char *)tmp_defn->view->st))
				eet(p->f, c, ENUM_ITEM_NAME_OVERLAP, 0);
		}

		// word
		//		,
		//		)
		//		expr
		c = absorb(p);
		if (c->code == PAR_R || c->code == COMMA) {
			defn->value = (void *)counter;
		} else {
			e = global_expression(p);

			if (e->code != CT_INT)
				eet(p->f, c, EXPECTED_INT_GLOB_EXPR, 0);
			defn->value = (void *)e->tvar->num;

			free_glob_expr(e);
			c = pser_cur(p);
		}

		if (c->code == COMMA)
			c = absorb(p);

		plist_add(p->enums, defn);
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
const char *const GLOBAL_VARS_NAMES_OVERLAP =
	"Переменная с таким именем уже существует.";
const char *const SUGGEST_RENAME_VAR = "изменить имя переменной";
const char *const SUGGEST_FIX_FUN_SIGNATURES_OVERLAP =
	"изменить типы аругментов функции";
const char *const SEVERAL_ARGS_CANT_SHARE_MEM =
	"Несколько аргументов объявленных таким образом не могут иметь синонимы с "
	"другими типом, так как они принадлежат к разным участкам памяти.";
const char *const SUGGEST_DELETE_ARGS_OR_COMMA =
	"удалить аргументы или запятую";
const char *const COMMA_ARGS_CAN_BE_ONLY_BY_ONE =
	"Аргументы для одного участка памяти могут быть только по одному, иначе "
	"это уже не один участок памяти.";
const char *const TOO_MUCH_ARGS_FOR_NOW =
	"Слишком много аргументов фукнции, на данный момент максимальное "
	"количество аргуентов функции: 7.";
const char *const SUGGEST_CUT_ARGS_SIZE = "уменьшить количество аргументов";
const char *const SUGGEST_CHANGE_ARG_TYPE_SIZE = "изменить размер типа";
const char *const ARR_AS_A_FUN_ARG_IS_PROHIBITED =
	"Массивы запрещены для передачи в аргументы функции напрямую.";
const char *const STRUCT_AS_A_FUN_ARG_IS_PROHIBITED =
	"Лики запрещены для передачи в аргументы функции напрямую.";
const char *const SUGGEST_CHANGE_TYPE_TO_A_PTR = "изменить тип на указатель";
const char *const GLOBAL_STRUCTS_NAMES_OVERLAP =
	"Лик с таким именем уже существует.";
const char *const SUGGEST_RENAME_STRUCT = "переименовать лик";

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
	colo_pos = p->pos;
	match(p, c, COLO);
	uc is_one_memory = args->size == 1;
	is_one_memories_flag = is_one_memory;

	type = type_expr(p);
	type_size = size_of_type(p, type);

	if (is_one_memory && from && (type_size != size_of_type(p, from->type)))
		eet(p->f, get_pser_token((p), -1), TYPES_SIZES_NOT_MATCH,
			SUGGEST_CHANGE_ARG_TYPE_SIZE);
	if (!is_one_memory && from)
		eet(p->f, get_pser_token((p), colo_pos - p->pos - 1),
			SEVERAL_ARGS_CANT_SHARE_MEM, SUGGEST_DELETE_ARGS_OR_COMMA);

	c = pser_cur(p);
	if (c->code == COMMA) {
		if (!is_one_memory)
			eet(p->f, c, SEVERAL_ARGS_CANT_SHARE_MEM,
				SUGGEST_DELETE_ARGS_OR_COMMA);
		consume(p); // consume ,

		// set thing to the single arg
		arg->offset = args_offset;
		arg->type = type;
		arg->arg_size = type_size;

		// get new arg
		eithers = parse_arg(p, arg, args_offset);
		if (!is_one_memories_flag && eithers->size != 1)
			eet(p->f, c, COMMA_ARGS_CAN_BE_ONLY_BY_ONE,
				SUGGEST_DELETE_ARGS_OR_COMMA);
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
	match(p, c, PAR_R);
}

// ### os explanation:
//   _ - name
//   _ - size
//   _ - mems size, usefull cuz u may need to know it, mems are not args
// ... - fields that are Arg's
enum IP_Code inst_pser_struct(struct Pser *p, struct PList *os) {
	long i, last_offset = -1, size = 0, mems_count = 0;
	struct Arg *arg;

	struct Inst *in;
	struct Token *c, *name = absorb(p); // skip лик
	expect(p, name, ID);
	plist_add(os, name); // struct name
	plist_add(os, 0);	 // reserved for size
	plist_add(os, 0);	 // reserved for mems_count

	for (i = 0; i < parsed_structs->size; i++) {
		in = plist_get(parsed_structs, i);
		c = plist_get(in->os, 0); // struct name token

		if (sc((char *)name->view->st, (char *)c->view->st))
			eet(p->f, name, GLOBAL_STRUCTS_NAMES_OVERLAP,
				SUGGEST_RENAME_STRUCT);
	}

	expect(p, absorb(p), PAR_L);
	parse_args(p, os);

	check_list_of_args_on_uniq_names(p->f, os, DCLR_STRUCT_ARGS);

	for (i = DCLR_STRUCT_ARGS; i < os->size; i++) {
		arg = plist_get(os, i);

		if (arg->offset != last_offset) {
			size += arg->arg_size;
			mems_count++;
		}

		last_offset = arg->offset;
	}

	plist_set(os, DCLR_STRUCT_SIZE, (void *)size);		 // set size
	plist_set(os, DCLR_STRUCT_MEMS, (void *)mems_count); // set mems_count

	return IP_DECLARE_STRUCT;
}

// ### os explanation:
//   _ - GlobVar with name and type
// ... - Arg's
//   0 - zero terminator
// ... - local inctrustions
enum IP_Code inst_pser_dare_fun(struct Pser *p, struct PList *os) {
	uint32_t i, j;
	long last_offset = -1;

	struct Arg *arg;
	struct TypeExpr *type;
	struct GlobVar *fun_variable = malloc(sizeof(struct GlobVar)), *tmp_var;
	struct TypeExpr *fun_type = new_type_expr(TC_FUN);
	fun_type->data.args_types = new_plist(2);

	struct Token *cur = absorb(p); // skip фц
	expect(p, cur, ID);
	plist_add(os, 0); // reserved place for variable
	fun_variable->name = cur;
	fun_variable->type = fun_type;
	fun_variable->value = 0; // os;

	cur = absorb(p);
	expect(p, cur, PAR_L);

	parse_args(p, os);
	for (i = 1; i < os->size; i++) {
		arg = plist_get(os, i);

		for (j = 0; j < arg->names->size; j++) {
			cur = plist_get(arg->names, j);
			check_list_of_vars_on_name(p, cur);

			plist_add(p->local_vars, new_plocal_var(cur, arg));
		}

		if (arg->type->code == TC_ARR)
			eet(p->f, cur, ARR_AS_A_FUN_ARG_IS_PROHIBITED,
				SUGGEST_CHANGE_TYPE_TO_A_PTR);
		if (arg->type->code == TC_STRUCT)
			eet(p->f, cur, STRUCT_AS_A_FUN_ARG_IS_PROHIBITED,
				SUGGEST_CHANGE_TYPE_TO_A_PTR);

		// it haves here types cuz fun type args are types
		// its needed for fun call
		// REMEMBER:
		// args cant have one offset cuz if they are then they
		// are one mem and will affect fun call that will be implemented only be
		// fun type and by not knowing fun args
		if (arg->offset != last_offset)
			plist_add(fun_type->data.args_types, arg->type);

		last_offset = arg->offset;
	}

	if (fun_type->data.args_types->size > MAX_ARGS_ON_REGISTERS)
		eet(p->f, fun_variable->name, TOO_MUCH_ARGS_FOR_NOW,
			SUGGEST_CUT_ARGS_SIZE);

	// if there is no type then its void type
	type = pser_cur(p)->code == PAR_L ? new_type_expr(TC_VOID) : type_expr(p);
	plist_add(fun_type->data.args_types, type);

	plist_set(os, 0, fun_variable);
	plist_add(os, 0); // args terminator

	// gen signature
	get_fun_signature_considering_args(os, fun_variable);
	fun_variable->value_label = 0;
	// check by signatures
	for (i = 0; i < p->global_vars->size; i++) {
		tmp_var = plist_get(p->global_vars, i);

		// cant use it here are_types_equal(tmp_var->type, fun_variable->type))
		if (tmp_var->type->code == TC_FUN &&
			sc((char *)tmp_var->signature->st,
			   (char *)fun_variable->signature->st))
			eet(p->f, fun_variable->name, FUN_SIGNATURES_OVERLAP,
				SUGGEST_FIX_FUN_SIGNATURES_OVERLAP);
	}

	// add after cuz if before then will compare with itself
	plist_add(p->global_vars, fun_variable);

	// body
	parse_block_of_local_inst(p, os);
	// free local vars
	plist_clear_items_free(p->local_vars);

	return IP_DECLARE_FUNCTION;
}

// ### os explanation:
// ... - GlobVar's
enum IP_Code inst_pser_global_let(struct Pser *p, struct PList *os) {
	struct PList *args;
	struct Arg *arg;
	struct GlobVar *var, *tmp_var;
	void *global_expr;
	uint32_t i = p->pos, j;

	consume(p); // skip пусть
	args = parse_arg(p, 0, 0);

	if (args->size != 1) {
		arg = plist_get(args, args->size - 1);
		// i dont beleive that its works but it should
		eet(p->f, plist_get(arg->names, arg->names->size - 1),
			SEVERAL_ARGS_CANT_SHARE_MEM, SUGGEST_DELETE_ARGS_OR_COMMA);
	}
	arg = plist_get(args, 0);

	// skip '='
	match(p, pser_cur(p), EQU);
	global_expr = parse_global_expression(p, arg->type);

	for (i = 0; i < arg->names->size; i++) {
		var = malloc(sizeof(struct GlobVar));

		var->name = plist_get(arg->names, i);
		var->type = arg->type;
		var->gvar_size = arg->arg_size;
		get_global_signature(var);
		var->value = global_expr;
		var->value_label = 0;

		for (j = 0; j < p->global_vars->size; j++) {
			tmp_var = plist_get(p->global_vars, j);

			// do i want to use here the name or the signature i dunno TODO: not
			// always woks as i wished
			if (sc((char *)tmp_var->signature->st, (char *)var->signature->st))
				eet(p->f, var->name, GLOBAL_VARS_NAMES_OVERLAP,
					SUGGEST_RENAME_VAR);
		}

		plist_add(p->global_vars, var);
		plist_add(os, var);
	}

	return IP_LET;
}
