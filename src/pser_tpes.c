#include "pser.h"

enum CE_Code are_types_compatible(struct TypeExpr *type, struct GlobExpr *e) {
	long some_value;
	struct TypeExpr *tmp_type;

	if (e->type)
		return CE_TODO;

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
		if (is_str_type(type))
			return CE_NONE;

		// assume array
		if (type->code != TC_ARR)
			return CE_STR_INCOMPATIBLE_TYPE;

		// assume uint8 array
		tmp_type = arr_type(type);
		if (tmp_type->code != TC_UINT8)
			return CE_STR_INCOMPATIBLE_TYPE;

		enum CE_Code ret_code = CE_NONE;

		// asume array size
		some_value = (long)arr_size(type);
		if (some_value != -1 && some_value != e->tvar->str->size)
			// TODO: provide len
			ret_code = CE_ARR_SIZES_DO_NOW_MATCH;

		// set size in any way
		some_value = e->tvar->view->size - 2;
		plist_set(type->data.arr, 1, (void *)some_value);

		return ret_code;
	}

	// here e->from != 0
	if (e->code == CT_GLOBAL_PTR) {
		if (type->code == TC_PTR)
			// if ptr targets are equal
			return are_types_compatible(type->data.ptr_target, e->from->value);

		if (type->code == TC_ARR)
			// if array items targets are equal
			return are_types_compatible(arr_type(type), e->from->value);

		return CE_TODO;
	}

	if (e->code == CT_GLOBAL)
		return CE_UNCOMPUTIBLE_DATA;

	return CE_NONE;
}
