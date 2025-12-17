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
constr CANT_BIN_ON_PTRS =
	"Нельзя применять бинарные операции к двум указателям.";
constr PTRS_TARGETS_WASNT_EQUAL = "Указываемые типы не были равны.";
constr UNEQUAL_TUPLES_ITEMS =
	"Количество назначаемых выражний в связке и количество "
	"возвращаемых выражений в связке не равны.";
constr EXPECTED_TUPLE_FOR_TUPLE_CALL_ASSIGN =
	"Ожидалась связка выражений, так как возвращаемый тип функции тоже "
	"сваязка выражений.";

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

	e->flags |= LEF_SIDE_EFFECT_FUN_CALL;

	for (i = 0; i < e->co.ops->size; i++) {
		call_arg_e = plist_get(e->co.ops, i);
		define_type_and_copy_flags_to_e(call_arg_e);
	}

	// REMEMBER: when call e->tvar->num = (long)fun gvar;
	if (!lceep(e->l, VAR)) {
	zero_fun_gvar_exit:
		e->type = copy_type_expr(find_return_type(e->l->type));
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
					e->type = copy_type_expr(find_return_type(gvar->type));
					e->r = (void *)gvar;
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

void define_numerous_call(struct LocalExpr *e) {
	struct LocalExpr *e1, *e2;
	u32 i;

	if (!lceep(e->l, VAR))
		eet(e->tvar, "copy_local_expr параша, вот", 0);
	if (!e->co.ops->size)
		eet(e->tvar, "вот зачем", 0);
	if (!e->tuple)
		e->tuple = new_plist(e->co.ops->size);

	for (i = 0; i < e->co.ops->size; i++) {
		e1 = plist_get(e->co.ops, i);

		if (lceep(e1, TUPLE)) {
			(e2 = e1)->code = LE_AFTER_CALL;
		} else {
			e2 = new_local_expr(LE_AFTER_CALL, 0, e->tvar);
			plist_add((e2->co.ops = new_plist(1)), e1);
		}
		e2->l = copy_local_expr(e->l), e2->r = 0;
		define_call_type(e2);
		plist_add(e->tuple, e2);
	}
	e->tuple->size--; // remove last e2 from tuple
	e2->tuple = e->tuple;
	paste_le(e, e2);
	free(e2);
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

	if (l_type == 0 || r_type == 0) {
		printf("[%s][%s]\n", vs(l->tvar), vs(r->tvar));
		eet(l->tvar, "эээ где типы", 0);
	}
	if (is_ptr_type(l_type) && is_ptr_type(r_type))
		eet(l->tvar, CANT_BIN_ON_PTRS, 0);

	if (is_ptr_type(l_type)) {
		res = copy_type_expr(l_type);
	} else if (is_ptr_type(r_type)) {
		res = copy_type_expr(r_type);
	} else if (is_real_type(l_type)) {
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

struct TypeExpr *terry_type(struct LocalExpr *e) {
	struct TypeExpr *lt = e->l->type, *rt = e->r->type;
	if (is_ptr_type(lt) && is_ptr_type(rt)) {
		if (are_types_equal(lt, rt))
			return copy_type_expr(lt);
		eet(e->r->tvar, PTRS_TARGETS_WASNT_EQUAL, 0);
	}
	if (is_ptr_type(lt)) {
		if (is_le_num(e->r, 0))
			return copy_type_expr(lt);
		eet(e->r->tvar, EXPECTED_PTR_TYPE, 0);
	}
	if (is_ptr_type(rt)) {
		if (is_le_num(e->l, 0))
			return copy_type_expr(rt);
		eet(e->l->tvar, EXPECTED_PTR_TYPE, 0);
	}
	return new_type_expr(max_code_or_any(lt->code, rt->code));
}

void define_le_type(struct LocalExpr *e) {
	if (e->type)
		return;

	u64 i;
	struct LocalExpr *other;

	if (is_bin_le(e)) {
		define_type_and_copy_flags_to_e(e->l);
		define_type_and_copy_flags_to_e(e->r);
		e->type = add_types(e->l, e->r);

	} else if (lce(BIN_TERRY)) {
		define_type_and_copy_flags_to_e(e->co.cond);
		define_type_and_copy_flags_to_e(e->l);
		define_type_and_copy_flags_to_e(e->r);
		e->type = terry_type(e);

	} else if (lce(BIN_ASSIGN)) {
		define_type_and_copy_flags_to_e(e->l);
		define_type_and_copy_flags_to_e(e->r);
		e->type = copy_type_expr(e->l->type);
		e->flags |= LEF_SIDE_EFFECT_MEMCH;

		if (e->r->type->code == TC_TUPLE) {
			if (!e->l->tuple)
				eet(e->tvar, EXPECTED_TUPLE_FOR_TUPLE_CALL_ASSIGN, 0);
			if (e->l->tuple->size + 1 != tup_itms(e->r->type)->size)
				eet(e->tvar, UNEQUAL_TUPLES_ITEMS, 0);
		}
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
			other = plist_get(e->co.ops, i);
			define_type_and_copy_flags_to_e(other);
		}
		e->type = 0;

	} else if (lcep(TUPLE)) {
		if (e->co.ops->size == 0)
			eet(e->tvar, USELESS_EMPTY_TUPLE, 0);
		// define type only for last as it will be used as expr,
		// and other exprs of yuple will be opted and compiled later
		other = plist_get(e->co.ops, e->co.ops->size - 1);
		define_type_and_copy_flags_to_e(other);
		// replace e and set tuple to it
		if (other->tuple)
			plat_plist(other->tuple, e->co.ops);
		else
			other->tuple = e->co.ops;
		paste_le(e, other);
		// remove other_epxr from tuple
		free(other);
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

	} else if (lcea(PIPE_LINE)) {
		define_type_and_copy_flags_to_e(e->r);

		e->code = LE_AFTER_CALL;
		if (lceep(e->l, TUPLE)) {
			e->co.ops = e->l->co.ops;
			free(e->l);
		} else {
			e->co.ops = new_plist(1);
			plist_add(e->co.ops, e->l);
		}
		e->l = e->r;
		e->r = 0;
		goto define_after_call;

	} else if (lce(NUMEROUS_CALL)) {
		define_numerous_call(e);
	} else if (lcea(CALL)) {
	define_after_call:
		define_call_type(e);

	} else if (lcea(INC) || lcea(DEC)) {
		define_type_and_copy_flags_to_e(e->l);
		e->flags |= LEF_SIDE_EFFECT_MEMCH;
		e->type = copy_type_expr(e->l->type);
	} else if (lcea(FIELD)) {
		if (lceeu(e->l, ADDR)) {
			e->l = e->l->l;
			e->code = LE_AFTER_FIELD_OF_PTR;
			blist_set(e->tvar->view, 1, '>');
		}
		define_struct_field_type_type(e);
	} else if (lcea(FIELD_OF_PTR)) {
		if (lceeu(e->l, AMPER)) {
			e->l = e->l->l;
			e->code = LE_AFTER_FIELD;
			blist_set(e->tvar->view, 1, '@');
		}
		define_struct_field_type_type(e);
	} else if (lcea(ENUM)) {
		define_enum(e);
	} else if (is_if(e)) {
		define_le_type(e->co.cond);
		if (lce(IF_ELIF))
			define_le_type(e->r);
	} else if (lce(RANGE_LOOP)) {
		define_le_type((struct LocalExpr *)e->tvar->num);
		define_le_type(e->l);
		define_le_type(e->r);
		if (e->co.cond)
			define_le_type(e->co.cond);
	} else
		exit(87);
}
