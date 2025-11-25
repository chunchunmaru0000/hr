#include "../gner/gner.h"
#include <stdio.h>

constr INVALID_TYPE_FOR_BIN = "Неверный тип операнда для бинарных операций, с "
							  "ними вообще только числовые типы используются.";
constr EXPECTED_STRUCT_PTR_TYPE =
	"Ожидалось выражение типа указателя на лик для обращения к его полю.";
constr EXPECTED_STRUCT_TYPE =
	"Ожидалось выражение типа лика для обращения к его полю.";
constr FIELD_NOT_FOUND_IN_STRUCT =
	"В лике не было найдено поле с таким именем.";
constr EXPECTED_FUN_TYPE =
	"Ожидалось выражение с типом функции, для её вызова.";
constr EXPECTED_PTR_TYPE = "Ожидалось выражение с типом указателя.";
constr USELESS_EMPTY_TUPLE =
	"Пустая связка выражений не имеет смысла, и вычислена быть не может.";
constr INDEX_SHOULD_BE_OF_INT_TYPE_ONLY =
	"Индекс может быть только типа целого числа.";
constr FUN_WITH_SUCH_ARGS_WASNT_FOUND =
	"Функция с сигнатурой, подходящей под данные типы аргументов, не была "
	"найдена.";
constr FUN_WITH_SUCH_NAME_WASNT_FOUND =
	"Именно функция с таким именем не была найдена.";

void define_var_type(struct LocalExpr *e) {
	struct LocalVar *lvar;
	struct GlobVar *gvar;

	if ((lvar = find_local_Var(ogner, e->tvar->view))) {
		e->type = copy_type_expr(lvar->type);
	} else if ((gvar = find_glob_Var(ogner, e->tvar->view))) {
		e->type = copy_type_expr(gvar->type);
		e->flags |= LEF_SIDE_EFFECT_GVAR;
	} else {
		eet(e->tvar, "ненененененененене", 0);
	}
}

void define_struct_field_type_type(struct LocalExpr *e) {
	struct Token *field_name = (struct Token *)e->r, *arg_field_name,
				 *struct_name;
	struct Inst *field_struct;
	struct BList *arg_field_full_name;
	struct Arg *arg;
	u32 i, j;

	define_type_and_copy_flags_to_e(e->l);

	if (lce(AFTER_FIELD_OF_PTR)) {
		if (e->l->type == 0 || e->l->type->code != TC_PTR ||
			e->l->type->data.ptr_target->code != TC_STRUCT)
			eet(e->tvar, EXPECTED_STRUCT_PTR_TYPE, 0);
		if ((field_struct =
				 find_struct(e->l->type->data.ptr_target->data.name)) == 0)
			exit(156);
	} else { // AFTER_FIELD
		if (e->l->type == 0 || e->l->type->code != TC_STRUCT)
			eet(e->tvar, EXPECTED_STRUCT_TYPE, 0);
		if ((field_struct = find_struct(e->l->type->data.name)) == 0)
			exit(157);
	}

	for (i = DCLR_STRUCT_ARGS; i < field_struct->os->size; i++) {
		arg = plist_get(field_struct->os, i);
		for (j = 0; j < arg->names->size; j++) {
			arg_field_name = plist_get(arg->names, j);

			if (sc(vs(field_name), vs(arg_field_name))) {
				struct_name = plist_get(field_struct->os, DCLR_STRUCT_NAME);

				arg_field_full_name = copy_str(struct_name->view);
				blist_add(arg_field_full_name, '.');
				blat_blist(arg_field_full_name, arg_field_name->view);
				zero_term_blist(arg_field_full_name);

				// REMEMBER: when field e->tvar->num = (long)arg;
				// and e->tvar->real = (double)(long)arg_field_full_name
				e->tvar->num = (long)arg;
				e->tvar->real = (double)(long)arg_field_full_name;
				e->type = copy_type_expr(arg->type);
				return;
			}
		}
	}

	eet(field_name, FIELD_NOT_FOUND_IN_STRUCT, 0);
}

// 3 basic types to compare:
// * * ptr/fun
// * * real
// * * integer
int good_enough_args_for_fun(struct PList *args_types, struct PList *ops) {
	u32 i, equ;
	struct TypeExpr *fa_type, *op_type;
	struct LocalExpr *operand_e;

	for (i = 0; i < ops->size; i++) {
		fa_type = plist_get(args_types, i);
		operand_e = plist_get(ops, i);
		op_type = operand_e->type;

		equ = 0;
		if (is_ptr_type(fa_type) && is_ptr_type(op_type)) {
			equ = are_types_equal(fa_type, op_type);
		} else if ((is_ptr_type(fa_type) && is_le_num(operand_e, 0))) {
			equ = 1;
		} else if (is_num_type(fa_type) && is_num_type(op_type))
			equ = 1;
		if (!equ)
			return 0;
	}
	return 1;
}

void define_call_type(struct LocalExpr *e) {
	struct LocalExpr *call_arg_e;
	struct SameNameFuns *snf;
	u32 i, j;

	define_type_and_copy_flags_to_e(e->l);
	if (e->l->type == 0 || e->l->type->code != TC_FUN)
		eet(e->l->tvar, EXPECTED_FUN_TYPE, 0);

	e->type = copy_type_expr(find_return_type(e->l->type));
	e->flags |= LEF_SIDE_EFFECT_FUN_CALL;

	for (i = 0; i < e->co.ops->size; i++) {
		call_arg_e = plist_get(e->co.ops, i);
		define_type_and_copy_flags_to_e(call_arg_e);
	}

	// REMEMBER: when call e->tvar->num = (long)fun gvar;
	if (!lceep(e->l, VAR)) {
	zero_fun_gvar_exit:
		e->tvar->num = 0;
		return;
	}
	declare_lvar_gvar;
	get_assignee_size(ogner, e->l, &gvar, &lvar);
	if (lvar || !gvar)
		goto zero_fun_gvar_exit;

	for (i = 0; i < ogner->same_name_funs->size; i++) {
		snf = plist_get(ogner->same_name_funs, i);

		// gvar->name just same name as e->l->tvar but shorter
		if (sc(bs(snf->name), vs(gvar->name))) {
			for (j = 0; j < snf->funs->size; j++) {
				gvar = plist_get(snf->funs, j);
				if (fun_args(gvar->type)->size - 1 == e->co.ops->size &&
					good_enough_args_for_fun(fun_args(gvar->type), e->co.ops)) {
					e->tvar->num = (long)gvar;
					return;
				}
			}
			eet(e->l->tvar, FUN_WITH_SUCH_ARGS_WASNT_FOUND, 0);
		}
	}
	eet(e->l->tvar, FUN_WITH_SUCH_NAME_WASNT_FOUND, 0);
}

void define_enum(struct LocalExpr *e) {
	struct Token *enum_item =
		find_enum_item(ogner->enums, e->tvar->view, e->l->tvar->view);
	if (enum_item == 0)
		eet(e->l->tvar, ENUM_ITEM_NOT_FOUND, 0);

	e->code = LE_PRIMARY_INT;
	e->type = new_type_expr(TC_I32);
	e->tvar->num = enum_item->num;

	blist_clear_free(e->tvar->view);
	e->tvar->view = int_to_str(e->tvar->num);

	free(e->l);
	e->l = 0;
}

enum TypeCode max_code_or_any(enum TypeCode c0, enum TypeCode c1) {
	enum TypeCode res = c0;
	if (c0 > c1) {
		res = c0;
	} else if (c1 > c0) {
		res = c1;
	}
	return res;
}

struct TypeExpr *add_types(struct LocalExpr *l, struct LocalExpr *r) {
	struct TypeExpr *l_type = l->type, *r_type = r->type, *res;

	if (l_type == 0 || !is_num_type(l_type))
		eet(l->tvar, INVALID_TYPE_FOR_BIN, 0);
	if (r_type == 0 || !is_num_type(r_type))
		eet(r->tvar, INVALID_TYPE_FOR_BIN, 0);

	if (is_real_type(l_type)) {
		if (is_real_type(r_type))
			res = new_type_expr(max_code_or_any(l_type->code, r_type->code));
		else
			res = new_type_expr(l_type->code);
	} else if (is_real_type(r_type)) {
		res = new_type_expr(r_type->code);
	} else { // both are ints or uints
		// turn int literal type to unsigned
		if (is_u_type(r->type->code) && lceep(l, INT))
			turn_type_to_simple(l, r->type->code);
		else if (is_u_type(l->type->code) && lceep(r, INT))
			turn_type_to_simple(r, l->type->code);

		res = new_type_expr(max_code_or_any(l_type->code, r_type->code));
	}
	return res;
}

void define_le_type(struct LocalExpr *e) {
	if (e->type)
		return;

	u64 i;
	struct LocalExpr *other_epxr;

	if (is_bin_le(e)) {
		define_type_and_copy_flags_to_e(e->l);
		define_type_and_copy_flags_to_e(e->r);
		e->type = add_types(e->l, e->r);

	} else if (lce(BIN_TERRY)) {
		define_type_and_copy_flags_to_e(e->l);
		define_type_and_copy_flags_to_e(e->r);
		define_type_and_copy_flags_to_e(e->co.cond);

	} else if (lce(BIN_ASSIGN)) {
		define_type_and_copy_flags_to_e(e->l);
		define_type_and_copy_flags_to_e(e->r);
		e->type = copy_type_expr(e->l->type);
		e->flags |= LEF_SIDE_EFFECT_MEMCH;

	} else if (lce(PRIMARY_INT)) {
		e->type = new_type_expr(TC_I32);
	} else if (lce(PRIMARY_REAL)) {
		e->type = new_type_expr(TC_DOUBLE);
	} else if (lce(PRIMARY_VAR)) {
		define_var_type(e);
	} else if (lce(PRIMARY_STR)) {
		e->type = new_type_expr(TC_ARR);
		e->type->data.arr = new_plist(2);
		plist_add(e->type->data.arr, new_type_expr(TC_U8));
		plist_add(e->type->data.arr, (void *)(i = e->tvar->str->size + 1));

	} else if (lce(PRIMARY_ARR)) {
		for (i = 0; i < e->co.ops->size; i++) {
			other_epxr = plist_get(e->co.ops, i);
			define_type_and_copy_flags_to_e(other_epxr);
		}
		e->type = 0;

	} else if (lcep(TUPLE)) {
		if (e->co.ops->size == 0)
			eet(e->tvar, USELESS_EMPTY_TUPLE, 0);
		// define type only for last as it will be used as expr,
		// and other exprs of yuple will be opted and compiled later
		other_epxr = plist_get(e->co.ops, e->co.ops->size - 1);
		define_type_and_copy_flags_to_e(other_epxr);
		// replace e and set tuple to it
		if (other_epxr->tuple)
			plat_plist(other_epxr->tuple, e->co.ops);
		else
			other_epxr->tuple = e->co.ops;
		paste_le(e, other_epxr);
		// remove other_epxr from tuple
		free(other_epxr);
		e->tuple->size--;

	} else if (is_unary(e) || lce(BOOL)) {
		define_type_and_copy_flags_to_e(e->l);

		if (lce(BOOL))
			e->type = new_type_expr(TC_I32);
		else if (lceu(ADDR)) {
			if (e->l->type->code != TC_PTR)
				eet(e->l->tvar, EXPECTED_PTR_TYPE, 0);
			e->type = copy_type_expr(e->l->type->data.ptr_target);
		} else if (lceu(AMPER)) {
			e->type = new_type_expr(TC_PTR);
			e->type->data.ptr_target = copy_type_expr(e->l->type);
		} else if (lceu(INC) || lceu(DEC)) {
			e->flags |= LEF_SIDE_EFFECT_MEMCH;
			e->type = copy_type_expr(e->l->type);
		} else
			e->type = copy_type_expr(e->l->type);

	} else if (lce(AFTER_INDEX)) {
		define_type_and_copy_flags_to_e(e->l);
		define_type_and_copy_flags_to_e(e->r);
		if (e->l->type == 0 ||
			(e->l->type->code != TC_ARR && e->l->type->code != TC_PTR))
			eet(e->l->tvar, ALLOWANCE_OF_INDEXATION, 0);
		e->type = e->l->type->code == TC_ARR
					  ? copy_type_expr(arr_type(e->l->type))
					  : copy_type_expr(ptr_targ(e->l->type));
		if (!is_int_type(e->r->type))
			eet(e->r->tvar, INDEX_SHOULD_BE_OF_INT_TYPE_ONLY, 0);

	} else if (lce(AFTER_PIPE_LINE)) {
		// TODO: (1, 2) |> x ? fa : fb
		define_type_and_copy_flags_to_e(e->r);

		e->code = LE_AFTER_CALL;
		if (e->l->code == LE_PRIMARY_TUPLE) {
			e->co.ops = e->l->co.ops;
			free(e->l);
		} else {
			e->co.ops = new_plist(1);
			plist_add(e->co.ops, e->l);
		}
		e->l = e->r;
		e->r = 0;
		goto define_after_call;

	} else if (lce(AFTER_CALL)) {
	define_after_call:
		define_call_type(e);

	} else if (lce(AFTER_INC) || lce(AFTER_DEC)) {
		define_type_and_copy_flags_to_e(e->l);
		e->flags |= LEF_SIDE_EFFECT_MEMCH;
		e->type = copy_type_expr(e->l->type);
	} else if (lce(AFTER_FIELD_OF_PTR) || lce(AFTER_FIELD)) {
		define_struct_field_type_type(e);
	} else if (lce(AFTER_ENUM)) {
		define_enum(e);
	}
}
