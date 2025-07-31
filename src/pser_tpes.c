#include "pser.h"

enum CE_Code are_types_compatible(struct TypeExpr *type, struct GlobExpr *e) {
	long some_value;
	struct TypeExpr *tmp_type;
	enum CE_Code tmp_code = CE_NONE;

	if (e->type) {
		// compares global types, not returns enum CE_Code, just boolean
		if (are_types_equal(type, e->type))
			return CE_NONE;
		return CE_TODO1;
	}

	if (e->code == CT_INT) {
		if (is_int_type(type))
			return CE_NONE;

		if (is_real_type(type)) {
			e->tvar->fpn = e->tvar->number;
			e->code = CT_REAL;
			return CE_NONE;
		}

		return CE_NUM_INCOMPATIBLE_TYPE;
	}

	if (e->code == CT_REAL) {
		if (is_real_type(type))
			return CE_NONE;

		if (is_int_type(type)) {
			e->tvar->number = e->tvar->fpn;
			e->code = CT_INT;
			return CE_NONE;
		}

		return CE_NUM_INCOMPATIBLE_TYPE;
	}

	if (e->code == CT_STR) {
		if (is_str_type(type)) // TC_VOID is considered in here
			return CE_NONE;

		// assume array
		if (type->code != TC_ARR)
			return CE_STR_INCOMPATIBLE_TYPE;

		// assume uint8 array
		tmp_type = arr_type(type);
		if (tmp_type->code != TC_UINT8)
			return CE_STR_INCOMPATIBLE_TYPE;

		// asume array size
		some_value = (long)arr_size(type);
		if (some_value != -1 && some_value != e->tvar->str->size)
			// TODO: provide len
			tmp_code = CE_ARR_SIZES_DO_NOW_MATCH;

		// set size in any way
		some_value = e->tvar->view->size - 2;
		// is arr type size is only for typization?
		plist_set(type->data.arr, 1, (void *)some_value);

		return tmp_code;
	}

	// here e->from != 0
	if (e->code == CT_GLOBAL_PTR) {
		// pointer can be taken only from an Identificator
		// so why do i even consider to compare its value type
		// so in here need to comapre e->from->type
		// where e->from->type is a pointed to type, not a pointer type

		if (is_ptr_type(type))
			if (are_types_equal(type->data.ptr_target, e->from->type))
				return CE_NONE;
		//
		// 		if (type->code == TC_PTR) {
		// 			if (type->data.ptr_target->code == TC_VOID ||
		// 				e->from->type->code == TC_VOID)
		// 				// void is compatible with any type
		// 				return CE_NONE;
		//
		// 			// if ptr targets are equal
		// 			// TODO: check this, i dunno what to use in here
		// 			// return are_types_compatible(type->data.ptr_target,
		// 			// e->from->value);
		// 			if (are_types_equal(type->data.ptr_target, e->from->type))
		// 				return CE_NONE;
		// 		}
		//
		// 		// see else
		// 		else if (type->code == TC_ARR) { // TODO: check here too
		// 			// if array target is equal to ptr target
		// 			if (are_types_compatible(arr_type(type), e->from->value))
		// 				return CE_NONE;
		// 		}
		return CE_PTR_INCOMPATIBLE_TYPE;
	}

	if (e->code == CT_GLOBAL)
		return CE_UNCOMPUTIBLE_DATA;

	if (e->code == CT_ARR)
		return CE_TODO3;

	if (e->code == CT_STRUCT)
		return CE_TODO4;

	// here e->from != 0
	if (e->code == CT_FUN) {
		// is it also includes is_void_ptr check in are_types_equal
		// if (is_void_ptr(type))
		// 	return CE_NONE;

		if (are_types_equal(type, e->from->type))
			return CE_NONE;

		return CE_FUN_INCOMPATIBLE_TYPE;
	}

	return CE_NONE;
}
