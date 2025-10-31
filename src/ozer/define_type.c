#include "../gner/gner.h"
#include <stdio.h>

constr EXPECTED_ARR_TYPE =
	"Ожидалось выражение типа массива для обращения к нему по индексу.";
constr EXPECTED_STRUCT_PTR_TYPE =
	"Ожидалось выражение типа указателя на лик для обращения к его полю.";
constr EXPECTED_STRUCT_TYPE =
	"Ожидалось выражение типа лика для обращения к его полю.";
constr FIELD_NOT_FOUND_IN_STRUCT =
	"В лике не было найдено поле с таким именем.";

void define_var_type(struct LocalExpr *e) {
	struct LocalVar *lvar;
	struct GlobVar *gvar;

	if ((lvar = find_local_Var(ogner, e->tvar->view))) {
		e->type = copy_type_expr(lvar->type);
	} else if ((gvar = find_glob_Var(ogner, e->tvar->view))) {
		e->type = copy_type_expr(gvar->type);
	} else if (0) {
		// TODO: find enum
	} else {
		eet(e->tvar, "ненененененененене", 0);
	}
}

void define_struct_field_type_type(struct LocalExpr *e) {
	struct Token *field_name = (struct Token *)e->r, *arg_field_name;
	struct Inst *field_struct;
	struct Arg *arg;
	u32 i, j;

	define_le_type(e->l);

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
				e->type = copy_type_expr(arg->type);
				return;
			}
		}
	}

	eet(field_name, FIELD_NOT_FOUND_IN_STRUCT, 0);
}

void define_le_type(struct LocalExpr *e) {
	if (e->type)
		return;

	u64 i;

	if (is_bin_le(e)) {
		define_le_type(e->l);
		define_le_type(e->r);

	} else if (lce(BIN_TERRY)) {
		define_le_type(e->l);
		define_le_type(e->r);
		define_le_type(e->co.cond);

	} else if (lce(BIN_ASSIGN)) {
		define_le_type(e->l);
		define_le_type(e->r);

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

	} else if (lce(PRIMARY_ARR) || lce(PRIMARY_TUPLE)) {
		e->type = 0;
		// LE_UNARY_MINUS
		// LE_UNARY_INC_BEFORE
		// LE_UNARY_DEC_BEFORE
		// LE_UNARY_NOT
		// LE_UNARY_BIT_NOT
		// LE_UNARY_AMPER
		// LE_UNARY_ADDR
	} else if (lce(AFTER_PIPE_LINE)) {

	} else if (lce(AFTER_INDEX)) {
		define_le_type(e->l);
		define_le_type(e->r);
		if (e->l->type == 0 || e->l->type->code != TC_ARR)
			eet(e->l->tvar, EXPECTED_ARR_TYPE, 0);
		e->type = copy_type_expr(arr_type(e->l->type));

	} else if (lce(AFTER_CALL)) {
		define_le_type(e->l);
		for (i = 0; i < e->co.ops->size; i++)
			define_le_type(plist_get(e->co.ops, i));
		e->type = copy_type_expr(arr_type(e->l->type));

	} else if (lce(AFTER_INC) || lce(AFTER_DEC)) {
		e->type = copy_type_expr(e->l->type);

	} else if (lce(AFTER_FIELD_OF_PTR) || lce(AFTER_FIELD)) {
		define_struct_field_type_type(e);
	}
}
