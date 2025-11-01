#include "../pser.h"
#include <stdint.h>

constr EXPECTED_STR_GLOB_EXPR_AS_ASM_ARG =
	"Глобальная инструкция ассемблера в качестве аргумента принимает только "
	"выражение строки или указателя на нее.";

// ### os explanation:
//   _ - assembly string token
enum IP_Code inst_pser_asm(struct Pser *p, struct PList *os) {
	struct Token *str_expr_token = absorb(p);

	struct GlobExpr *str = global_expression(p);
	if (str->code != CT_STR && str->code != CT_STR_PTR)
		eet(str_expr_token, EXPECTED_STR_GLOB_EXPR_AS_ASM_ARG, 0);

	str_expr_token = str->tvar;
	free(str);

	plist_add(os, str_expr_token);
	return IP_ASM;
}

constr ENUM_NAME_OVERLAP = "Счет с таким же именем уже существует.";
constr ENUM_ITEM_NAME_OVERLAP =
	"Элемент данного счета с таким же именем уже существует.";
constr EXPECTED_INT_GLOB_EXPR =
	"Результатом глобального выражения для значения счета может быть только "
	"целое число.";

// ### os explanation:
//   _ - struct Enum *enum_obj
enum IP_Code inst_pser_enum(struct Pser *p, struct PList *os) {
	struct GlobExpr *e;
	struct Token *c, *other_name, *item_name;
	long counter = 0;
	u32 i;

	struct Enum *enum_obj = malloc(sizeof(struct Enum)), *tmp_enum;
	enum_obj->enum_name = absorb(p);
	expect(enum_obj->enum_name, ID);
	enum_obj->items = new_plist(4);

	foreach_begin(tmp_enum, p->enums);
	if (sc(vs(enum_obj->enum_name), vs(tmp_enum->enum_name)))
		eet(c, ENUM_NAME_OVERLAP, 0);
	foreach_end;

	plist_add(os, enum_obj);
	plist_add(p->enums, enum_obj);

	c = absorb(p);
	expect(c, PAR_L);

	for (c = absorb(p); not_ef_and(PAR_R, c);) {
		expect(c, ID);
		item_name = c;

		// check here for identical names in enums items
		foreach_begin(other_name, enum_obj->items);
		if (vc(item_name, other_name))
			eet(item_name, ENUM_ITEM_NAME_OVERLAP, 0);
		foreach_end;
		plist_add(enum_obj->items, item_name);

		c = absorb(p);
		if (c->code == PAR_R || c->code == COMMA) {
			item_name->num = counter;
		} else {
			e = global_expression(p);

			if (e->code != CT_INT)
				eet(c, EXPECTED_INT_GLOB_EXPR, 0);
			item_name->num = e->tvar->num;

			free_glob_expr(e);
			c = pser_cur(p);
		}
		if (c->code == COMMA)
			c = absorb(p);

		counter++;
	}
	match(c, PAR_R);

	return IP_DECLARE_ENUM;
}

constr TYPES_SIZES_NOT_MATCH =
	"Размеры типов для одного участка памяти должны быть одинаковы.";
constr FUN_SIGNATURES_OVERLAP =
	"Сигнатура данной функции повторяет сигнатуру другой уже объявленной "
	"функции, даже если типы записаны по разному но равны по смыслу(например "
	"'*ц8' и 'стр'), их сигнатуры будут равны.";
constr GLOBAL_VARS_NAMES_OVERLAP = "Переменная с таким именем уже существует.";
constr SUGGEST_RENAME_VAR = "изменить имя переменной";
constr SUGGEST_FIX_FUN_SIGNATURES_OVERLAP = "изменить типы аругментов функции";
constr SEVERAL_ARGS_CANT_SHARE_MEM =
	"Несколько аргументов объявленных таким образом не могут иметь синонимы с "
	"другими типом, так как они принадлежат к разным участкам памяти.";
constr SUGGEST_DELETE_ARGS_OR_COMMA = "удалить аргументы или запятую";
constr COMMA_ARGS_CAN_BE_ONLY_BY_ONE =
	"Аргументы для одного участка памяти могут быть только по одному, иначе "
	"это уже не один участок памяти.";
constr TOO_MUCH_ARGS_FOR_NOW =
	"Слишком много аргументов фукнции, на данный момент максимальное "
	"количество аргуентов функции: 7.";
constr SUGGEST_CUT_ARGS_SIZE = "уменьшить количество аргументов";
constr SUGGEST_CHANGE_ARG_TYPE_SIZE = "изменить размер типа";
constr ARR_AS_A_FUN_ARG_IS_PROHIBITED =
	"Массивы запрещены для передачи в аргументы функции напрямую, надо "
	"использовать указатель.";
constr STRUCT_AS_A_FUN_ARG_IS_PROHIBITED =
	"Лики запрещены для передачи в аргументы функции напрямую.";
constr SUGGEST_CHANGE_TYPE_TO_A_PTR = "изменить тип на указатель";
constr GLOBAL_STRUCTS_NAMES_OVERLAP = "Лик с таким именем уже существует.";
constr SUGGEST_RENAME_STRUCT = "переименовать лик";
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
		eet(get_pser_token((p), -1), TYPES_SIZES_NOT_MATCH,
			SUGGEST_CHANGE_ARG_TYPE_SIZE);
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

// ### os explanation:
//   _ - name: struct Token *
//   _ - size
//   _ - mems size, usefull cuz u may need to know it, mems are not args
// ... - fields that are Arg's
enum IP_Code inst_pser_struct(struct Pser *p, struct PList *os) {
	os->cap_pace = 16;

	long i, last_offset = -1, size = 0, mems_count = 0;
	struct Arg *arg;

	struct Inst *in;
	struct Token *c, *name = absorb(p); // skip лик
	expect(name, ID);
	plist_add(os, name); // struct name
	plist_add(os, 0);	 // reserved for size
	plist_add(os, 0);	 // reserved for mems_count

	for (i = 0; i < parsed_structs->size; i++) {
		in = plist_get(parsed_structs, i);
		c = plist_get(in->os, 0); // struct name token

		if (vc(name, c))
			eet(name, GLOBAL_STRUCTS_NAMES_OVERLAP, SUGGEST_RENAME_STRUCT);
	}

	expect(absorb(p), PAR_L);
	parse_args(p, os);

	check_list_of_args_on_uniq_names(os, DCLR_STRUCT_ARGS);

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
	expect(cur, ID);
	plist_add(os, 0); // reserved place for variable
	fun_variable->name = cur;
	fun_variable->type = fun_type;
	fun_variable->value = 0; // os;

	cur = absorb(p);
	expect(cur, PAR_L);

	parse_args(p, os);
	for (i = 1; i < os->size; i++) {
		arg = plist_get(os, i);

		for (j = 0; j < arg->names->size; j++) {
			cur = plist_get(arg->names, j);
			check_list_of_vars_on_name(p, cur);

			plist_add(p->local_vars, new_plocal_var(cur, arg));
		}

		if (arg->type->code == TC_ARR)
			eet(cur, ARR_AS_A_FUN_ARG_IS_PROHIBITED,
				SUGGEST_CHANGE_TYPE_TO_A_PTR);
		if (arg->type->code == TC_STRUCT)
			eet(cur, STRUCT_AS_A_FUN_ARG_IS_PROHIBITED,
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
		eet(fun_variable->name, TOO_MUCH_ARGS_FOR_NOW, SUGGEST_CUT_ARGS_SIZE);

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
			eet(fun_variable->name, FUN_SIGNATURES_OVERLAP,
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
	struct GlobExpr *global_expr;
	uint32_t i = p->pos, j;

	consume(p); // skip пусть
	args = parse_arg(p, 0, 0);

	if (args->size != 1) {
		arg = plist_get(args, args->size - 1);
		// i dont beleive that its works but it should
		eet(plist_get(arg->names, arg->names->size - 1),
			SEVERAL_ARGS_CANT_SHARE_MEM, SUGGEST_DELETE_ARGS_OR_COMMA);
	}
	arg = plist_get(args, 0);

	// skip '='
	match(pser_cur(p), EQU);
	global_expr = parse_global_expression(p, arg->type);

	for (i = 0; i < arg->names->size; i++) {
		var = malloc(sizeof(struct GlobVar));

		var->name = plist_get(arg->names, i);
		var->type = arg->type;
		var->gvar_size = size_of_type(p, global_expr->type);
		get_global_signature(var);
		var->value = global_expr;
		var->value_label = 0;

		for (j = 0; j < p->global_vars->size; j++) {
			tmp_var = plist_get(p->global_vars, j);

			if (tmp_var->type->code == TC_ARR) {
				if (sc(vs(tmp_var->name), vs(var->name)))
					eet2(var->name, tmp_var->name, GLOBAL_VARS_NAMES_OVERLAP,
						 SUGGEST_RENAME_VAR);
			} else {
				if (sc(bs(tmp_var->signature), bs(var->signature)))
					eet2(var->name, tmp_var->name, GLOBAL_VARS_NAMES_OVERLAP,
						 SUGGEST_RENAME_VAR);
			}
		}

		plist_add(p->global_vars, var);
		plist_add(os, var);
	}

	return IP_LET;
}
