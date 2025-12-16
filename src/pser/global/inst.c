#include "../pser.h"
#include <stdint.h>
#include <stdio.h>

constr EXPECTED_STR_GLOB_EXPR_AS_ASM_ARG =
	"Глобальная инструкция ассемблера в качестве аргумента принимает только "
	"выражение строки или указателя на нее.";
struct PList *parse_arg(struct Pser *p, struct Arg *from, long args_offset);

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

constr FUN_SIGNATURES_OVERLAP =
	"Сигнатура данной функции повторяет сигнатуру другой уже объявленной "
	"функции, даже если типы записаны по разному но равны по смыслу(например "
	"'*ц8' и 'стр'), их сигнатуры будут равны.";
constr STRUCT_AS_A_FUN_ARG_IS_PROHIBITED =
	"Лики запрещены для передачи в аргументы функции напрямую.";
constr SUGGEST_CHANGE_TYPE_TO_A_PTR = "изменить тип на указатель";
constr GLOBAL_STRUCTS_NAMES_OVERLAP = "Лик с таким именем уже существует.";
constr SUGGEST_RENAME_STRUCT = "переименовать лик";
constr ARR_AS_A_FUN_ARG_IS_PROHIBITED =
	"Массивы запрещены для передачи в аргументы функции напрямую, надо "
	"использовать указатель.";
constr SUGGEST_RENAME_VAR = "изменить имя переменной";
constr SUGGEST_FIX_FUN_SIGNATURES_OVERLAP = "изменить типы аругментов функции";
constr GLOBAL_VARS_NAMES_OVERLAP = "Переменная с таким именем уже существует.";
constr TOO_MUCH_ARGS_FOR_NOW =
	"Слишком много аргументов фукнции, на данный момент максимальное "
	"количество аргуентов функции: 7.";
constr SUGGEST_CUT_ARGS_SIZE = "уменьшить количество аргументов";

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

	plist_set(os, DCLR_STRUCT_SIZE, (void *)size); // set size
	// plist_set(os, DCLR_STRUCT_SIZE,
	// 		  (void *)(size + 16 - (size % 16)));		 // set size
	plist_set(os, DCLR_STRUCT_MEMS, (void *)mems_count); // set mems_count

	return IP_DECLARE_STRUCT;
}

constr FF_DO_NOT_CREATE_STACK_FRAME = "__АСМ";

// ### os explanation:
//   _ - GlobVar with name and type
// ... - Arg's
//   0 - zero terminator
// ... - local inctrustions
enum IP_Code inst_pser_dare_fun(struct Pser *p, struct PList *os) {
	u32 i, j;
	long last_offset = -1;

	struct SameNameFuns *snf;
	struct Arg *arg;
	struct TypeExpr *type;
	struct GlobVar *fun_variable = malloc(sizeof(struct GlobVar)), *tmp_var;
	struct TypeExpr *fun_type = new_type_expr(TC_FUN);
	u64 fun_flags = 0;
	struct Token *cur;

	fun_type->data.args_types = new_plist(2);

	for (cur = absorb(p); 1; cur = absorb(p)) { // skip фц
		if (sc(vs(cur), FF_DO_NOT_CREATE_STACK_FRAME)) {
			fun_flags |= DO_NOT_CREATE_STACK_FRAME;
			continue;
		}
		break;
	}
	expect(cur, ID);
	plist_add(os, 0); // reserved place for variable
	fun_variable->name = cur;
	fun_variable->name->num = fun_flags;
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
	for (i = 0; i < p->same_name_funs->size; i++) {
		snf = plist_get(p->same_name_funs, i);

		if (sc(bs(snf->name), vs(fun_variable->name))) {
			for (j = 0; j < snf->funs->size; j++) {
				tmp_var = plist_get(snf->funs, j);
				if (sc(bs(tmp_var->signature), bs(fun_variable->signature)))
					eet(fun_variable->name, FUN_SIGNATURES_OVERLAP,
						SUGGEST_FIX_FUN_SIGNATURES_OVERLAP);
			}
			// if here then means that valid signature of same name
			goto add_fun_variable_to_same_name_funs;
		}
	}
	snf = malloc(sizeof(struct SameNameFuns));
	snf->name = fun_variable->name->view;
	snf->funs = new_plist(2);
	plist_add(p->same_name_funs, snf);

add_fun_variable_to_same_name_funs:
	plist_add(snf->funs, fun_variable);

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
