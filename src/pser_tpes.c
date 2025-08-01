#include "pser.h"

void are_types_compatible(struct PList *msgs, struct TypeExpr *type,
						  struct GlobExpr *e) {
	long n;
	struct TypeExpr *tmp_type;
	enum CE_Code tmp_code = CE_NONE;

	if (e->type) {
		// compares global types, not returns enum CE_Code, just boolean
		if (!are_types_equal(type, e->type)) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_TODO1);
		}

		return;
	}

	if (e->code == CT_INT) {
		if (is_int_type(type))
			return;

		if (is_real_type(type)) {
			e->tvar->fpn = e->tvar->number;
			e->code = CT_REAL;
			return;
		}

		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_NUM_INCOMPATIBLE_TYPE);
		return;
	}

	if (e->code == CT_REAL) {
		if (is_real_type(type))
			return;

		if (is_int_type(type)) {
			e->tvar->number = e->tvar->fpn;
			e->code = CT_INT;
			return;
		}

		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_NUM_INCOMPATIBLE_TYPE);
		return;
	}

	if (e->code == CT_STR) {
		// TC_VOID is considered in here
		if (is_str_type(type))
			return;

		// assume array
		if (type->code != TC_ARR) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_STR_INCOMPATIBLE_TYPE);
			return;
		}

		// assume uint8 array
		tmp_type = arr_type(type);
		if (tmp_type->code != TC_UINT8) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_STR_INCOMPATIBLE_TYPE);
			return;
		}

		// asume array size
		n = (long)arr_size(type);
		if (n != -1 && n != e->tvar->str->size)
			// TODO: provide len
			tmp_code = CE_ARR_SIZES_DO_NOW_MATCH;

		// set size in any way
		n = e->tvar->view->size - 2;
		// is arr type size is only for typization?
		plist_set(type->data.arr, 1, (void *)n);

		plist_add(msgs, (void *)tmp_code);
		return;
	}

	// here e->from != 0
	if (e->code == CT_GLOBAL_PTR) {
		// pointer can be taken only from an Identificator
		// so why do i even consider to compare its value type
		// so in here need to comapre e->from->type
		// where e->from->type is a pointed to type, not a pointer type

		if (is_ptr_type(type)) {
			// wrap around e->from->type
			tmp_type = new_type_expr(TC_PTR);
			// tmp_type = &(struct TypeExpr){TC_PTR, 0};
			tmp_type->data.ptr_target = e->from->type;

			if (!are_types_equal(type, tmp_type)) {
				free(tmp_type);
				plist_add(msgs, e->tvar);
				plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
				return;
			}

			free(tmp_type);
			return;
		}

		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
		return;
	}

	if (e->code == CT_GLOBAL) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_UNCOMPUTIBLE_DATA);
		return;
	}

	if (e->code == CT_ARR) {
		if (!is_arr_type(type)) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_ARR_INCOMPATIBLE_TYPE);
			return;
		}

		if (type->code == TC_ARR) {
			for (n = 0; n < e->globs->size; n++) {
				// TODO: provide n
				are_types_compatible(msgs, arr_type(type),
									 plist_get(e->globs, n));
			}
		} else {
			for (n = 0; n < e->globs->size; n++) {
				// TODO: provide n
				are_types_compatible(msgs, ptr_targ(type),
									 plist_get(e->globs, n));
			}
		}
		return;
	}

	if (e->code == CT_STRUCT) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_TODO3);
		return;
	}

	// here e->from != 0
	if (e->code == CT_FUN) {
		// is it also includes is_void_ptr check in are_types_equal
		if (!are_types_equal(type, e->from->type)) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_FUN_INCOMPATIBLE_TYPE);
		}

		return;
	}

	plist_add(msgs, e->tvar);
	plist_add(msgs, (void *)CE_TODO4);
}
