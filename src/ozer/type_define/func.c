#include "../../gner/gner.h"

constr EXPECTED_FUN_TYPE =
	"Ожидалось выражение с типом функции, для её вызова.";
constr FUN_WITH_SUCH_ARGS_WASNT_FOUND =
	"Функция с сигнатурой, подходящей под данные типы аргументов, не была "
	"найдена.";
constr FUN_WITH_SUCH_NAME_WASNT_FOUND =
	"Именно функция с таким именем не была найдена.";

// 2 basic types to compare:
// * * ptr/fun
// * * number
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

// 3 basic types to compare:
// * * ptr/fun
// * * real
// * * integer
int just_same_types_of_args(struct PList *args_types, struct PList *ops) {
	u32 i;
	struct TypeExpr *fa_type, *op_type;

	for (i = 0; i < ops->size; i++) {
		fa_type = plist_get(args_types, i);
		op_type = ((struct LocalExpr *)plist_get(ops, i))->type;

		if ((is_ptr_type(fa_type) || is_ptr_type(op_type)) &&
			are_types_equal(fa_type, op_type))
			continue;
		else if (is_real_type(fa_type) && is_real_type(op_type))
			continue;
		else if (is_num_int_type(fa_type) && is_num_int_type(op_type))
			continue;
		return 0;
	}
	return 1;
}

struct SameNameFuns *find_same_name_funs(struct BList *name) {
	for (u32 i = 0; i < ogner->same_name_funs->size; i++) {
		struct SameNameFuns *snf = plist_get(ogner->same_name_funs, i);
		if (sc(bs(snf->name), bs(name)))
			return snf;
	}
	return 0;
}

void define_call_type(struct LocalExpr *e) {
	struct LocalExpr *call_arg_e;
	u32 i;

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

	struct SameNameFuns *snf;
	// gvar->name just same name as e->l->tvar but shorter
	if ((snf = find_same_name_funs(gvar->name->view)) == 0)
		eet(e->l->tvar, FUN_WITH_SUCH_NAME_WASNT_FOUND, 0);

	for (i = 0; i < snf->funs->size; i++) {
		gvar = plist_get(snf->funs, i);
		if (fun_args(gvar->type)->size - 1 == e->co.ops->size &&
			just_same_types_of_args(fun_args(gvar->type), e->co.ops))
			goto fun_args_found;
	}
	for (i = 0; i < snf->funs->size; i++) {
		gvar = plist_get(snf->funs, i);
		if (fun_args(gvar->type)->size - 1 == e->co.ops->size &&
			good_enough_args_for_fun(fun_args(gvar->type), e->co.ops))
			goto fun_args_found;
	}
	eet(e->l->tvar, FUN_WITH_SUCH_ARGS_WASNT_FOUND, 0);

fun_args_found:
	e->type = copy_type_expr(find_return_type(gvar->type));
	e->r = (void *)gvar;
}

void define_numerous_call(struct LocalExpr *e) {
	struct LocalExpr *e1, *e2;
	u32 i;

	if (!lceep(e->l, VAR))
		eet(e->tvar, "copy_local_expr лее", 0);
	if (!e->co.ops->size)
		eet(e->tvar, "вот ээээ", 0);
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
