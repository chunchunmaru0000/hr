#include "pser.h"
#include <stdio.h>

void cmpt_int(struct PList *msgs, struct TypeExpr *type, struct GlobExpr *e) {
	if (is_int_type(type))
		return;

	if (is_real_type(type)) {
		e->tvar->real = e->tvar->num;
		e->code = CT_REAL;
		return;
	}

	plist_add(msgs, e->tvar);
	plist_add(msgs, (void *)CE_NUM_INCOMPATIBLE_TYPE);
}

void cmpt_real(struct PList *msgs, struct TypeExpr *type, struct GlobExpr *e) {
	if (is_real_type(type))
		return;

	if (is_int_type(type)) {
		e->tvar->num = e->tvar->real;
		e->code = CT_INT;
		return;
	}

	plist_add(msgs, e->tvar);
	plist_add(msgs, (void *)CE_NUM_INCOMPATIBLE_TYPE);
}

void cmpt_global_ptr(struct PList *msgs, struct TypeExpr *type,
					 struct GlobExpr *e) {
	// here e->from != 0
	// pointer can be taken only from an Identificator
	// so why do i even consider to compare its value type
	// so in here need to comapre e->from->type
	// where e->from->type is a pointed to type, not a pointer type

	if (!is_ptr_type(type)) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
		return;
	}

	// wrap around e->from->type
	struct TypeExpr *tmp_type =
		e->not_from_child
			? e->from->type
			: &(struct TypeExpr){TC_PTR, {.ptr_target = e->from->type}};

	// printf("%s %d\n", vs(e->tvar), e->not_from_child);
	if (!are_types_equal(type, tmp_type)) {
		// printf("tmp_type = %d %d\n", tmp_type->code,
		//         tmp_type->data.ptr_target->code);
		// printf("    type = %d %d\n", type->code,
		//         type->data.ptr_target->code);

		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
	}
}

void cmpt_global(struct PList *msgs, struct TypeExpr *type,
				 struct GlobExpr *e) {
	if (e->not_from_child) {
		e->code = CT_GLOBAL_PTR;
		cmpt_global_ptr(msgs, type, e);
		return;
	}
	plist_add(msgs, e->tvar);
	plist_add(msgs, (void *)CE_UNCOMPUTIBLE_DATA);
}

void cmpt_str(struct PList *msgs, struct TypeExpr *type, struct GlobExpr *e) {
	long arr_items;

	if (type->code == TC_PTR) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_STR_IS_NOT_A_PTR);
		return;
	}
	// assume array
	if (type->code != TC_ARR) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_STR_INCOMPATIBLE_TYPE);
		return;
	}

	// assume uint8 array
	if (arr_type(type)->code != TC_UINT8) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_STR_INCOMPATIBLE_TYPE);
		return;
	}

	// asume array size
	arr_items = (long)arr_len(type);

	if (arr_items == -1) { // need to adjust it by size of e->globs->size
		arr_items = e->tvar->str->size + 1;
		plist_set(type->data.arr, 1, (void *)arr_items);
		goto valid_str_as_arr_size;
	}
	if (e->tvar->str->size + 1 > arr_items) {
		plist_add(msgs, (void *)arr_items);
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_TOO_MUCH_CHARS_FOR_THIS_STR);
		return;
	}
	if (arr_items > e->tvar->str->size + 1) {
		plist_add(msgs, (void *)arr_items);
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_TOO_LESS_CHARS_FOR_THIS_STR);

		e->tvar->view->size--;
		for (; e->tvar->str->size + 1 < arr_items;) {
			blist_add(e->tvar->view, '\\');
			blist_add(e->tvar->view, '0'); // \0
			blist_add(e->tvar->str, 0);
		}
		blist_add(e->tvar->view, '"');
		convert_blist_to_blist_from_str(e->tvar->view);
		convert_blist_to_blist_from_str(e->tvar->str);
	}

valid_str_as_arr_size:
	// set size in any way
	arr_items = e->tvar->str->size + 1; // + 1 cuz '\0'
	set_arr_len(type->data.arr, arr_items);

	plist_add(msgs, e->tvar);
	plist_add(msgs, (void *)CE_NONE);
}
