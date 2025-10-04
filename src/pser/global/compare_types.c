#include "../pser.h"
#include <stdio.h>

#define add_err(err)                                                           \
	do {                                                                       \
		plist_add(msgs, e->tvar);                                              \
		plist_add(msgs, (void *)(err));                                        \
	} while (0)

void check_type_on_struct_fields(struct PList *, struct TypeExpr *,
								 struct GlobExpr *);

struct GlobExpr *new_zero_type(struct TypeExpr *type, int size,
							   struct Token *tvar) {
	struct GlobExpr *e = malloc(sizeof(struct GlobExpr));

	e->code = CT_ZERO;
	e->from = 0;
	e->type = copy_type_expr(type);

	// not used for defining value, just in case here
	e->tvar = malloc(sizeof(struct Token));
	e->tvar->view = tvar->view;
	e->tvar->num = size;

	e->globs = 0;

	return e;
}

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

// here e->type != 0
void cmpt_fun(struct PList *msgs, struct TypeExpr *type, struct GlobExpr *e) {
	// is it also includes is_void_ptr check in are_types_equal
	if (!are_types_equal(type, e->from->type)) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_FUN_INCOMPATIBLE_TYPE);
	}
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

void cmpt_struct_ptr(struct PList *msgs, struct TypeExpr *type,
					 struct GlobExpr *e) {
	if (!is_ptr_type(type)) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
		return;
	}
	if (is_void_ptr(type)) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_CANT_DEFINE_STRUCT_TYPE);
		return;
	}
	check_type_on_struct_fields(msgs, ptr_targ(type), e);
}
void cmpt_struct(struct PList *msgs, struct TypeExpr *type,
				 struct GlobExpr *e) {
	if (e->from) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_STRUCT_FROM_OTHER_GLOBAL_STRUCT);
		return;
	}
	check_type_on_struct_fields(msgs, type, e);
}

void check_named_fields(struct PList *msgs, struct GlobExpr *e,
						struct PList *lik_os);
void check_type_on_struct_fields(struct PList *msgs, struct TypeExpr *type,
								 struct GlobExpr *e) {
	struct GlobExpr *glob;
	long lik_mems;
	uint32_t i;

	if (type->code != TC_STRUCT) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_STRUCT_INCOMPATIBLE_TYPE);
		return;
	}

	struct PList *lik_os = find_lik_os(type->data.name);
	// this check can be deleted cuz when parse a type then it already
	// checks for existence of the struct
	if (lik_os == 0) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_STRUCT_WASNT_FOUND);
		return;
	}

	lik_mems = (long)plist_get(lik_os, DCLR_STRUCT_MEMS);

	if (e->globs->size > lik_mems) {
		plist_add(msgs, (void *)lik_mems);
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_TOO_MUCH_FIELDS_FOR_THIS_STRUCT);
		return;
	}

	if (!lik_mems)
		return;
	if (e->struct_with_fields) {
		check_named_fields(msgs, e, lik_os);
		return;
	}

	struct Arg *arg;
	if (lik_mems > e->globs->size) {
		plist_add(msgs, (void *)lik_mems);
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_TOO_LESS_FIELDS_FOR_THIS_STRUCT);

		// get first arg
		arg = get_arg_by_mem_index(lik_os, e->globs->size);
		plist_add(e->globs, new_zero_type(arg->type, arg->arg_size, e->tvar));
		// get all other args
		for (; e->globs->size < lik_mems;) {
			arg = get_arg_of_next_offset(lik_os, arg->offset);
			plist_add(e->globs,
					  new_zero_type(arg->type, arg->arg_size, e->tvar));
		}
	}

	if (e->globs->size == 0) // zero args in struct in any case
		return;

	for (i = 0; i < e->globs->size; i++) {
		glob = plist_get(e->globs, i);
		arg = get_arg_by_mem_index(lik_os, i);

		if (glob->code == CT_ZERO)
			continue;

		are_types_compatible(msgs, arg->type, glob);

		if (glob->type)
			free_type(glob->type);
		glob->type = copy_type_expr(arg->type);
	}
}

int find_field_in_arg(struct PList *msgs, struct GlobExpr *e,
					  struct PList *fields, struct Arg *arg) {
	struct NamedStructField *field;
	struct Token *arg_name;
	uint32_t i, j;

	for (i = 0; i < fields->size; i++) {
		field = plist_get(fields, i);
		if (!field) // if not already checked
			continue;

		for (j = 0; j < arg->names->size; j++) {
			arg_name = plist_get(arg->names, j);
			if (!sc(vs(arg_name), vs(field->name_token)))
				continue;

			are_types_compatible(msgs, arg->type, field->expression);
			plist_add(e->globs, field->expression);

			if (field->expression->type)
				free_type(field->expression->type);
			field->expression->type = copy_type_expr(arg->type);

			free(field);
			plist_set(fields, i, 0);
			return 1;
		}
	}
	return 0;
}

void check_named_fields(struct PList *msgs, struct GlobExpr *e,
						struct PList *lik_os) {
	struct PList *fields = e->globs;
	struct NamedStructField *field;
	struct Arg *arg, *next_arg;
	uint32_t i, mem_count = 0;
	long last_mem_offset = -1;

	e->globs = new_plist(fields->size);

	next_arg = plist_get(lik_os, DCLR_STRUCT_ARGS);
	for (i = DCLR_STRUCT_ARGS; i < lik_os->size; i++) {
		arg = next_arg;

		if (last_mem_offset != arg->offset) {
			last_mem_offset = arg->offset;
			mem_count++;
		}

		next_arg = i + 1 < lik_os->size ? plist_get(lik_os, i + 1) : 0;
		if (e->globs->size == mem_count) // skip other mems if mem already found
			continue;

#define last_arg_of_mem() ((!next_arg) || next_arg->offset != arg->offset)
		if (!find_field_in_arg(msgs, e, fields, arg) && last_arg_of_mem())
			plist_add(e->globs,
					  new_zero_type(arg->type, arg->arg_size, e->tvar));
	}

	for (i = 0; i < fields->size; i++) {
		field = plist_get(fields, i);
		if (field) {
			plist_add(msgs, field->name_token);
			plist_add(msgs, (void *)CE_EXCESSING_FIELD);
			free(field);
		}
	}
	plist_free(fields);
}

void cmpt_str_ptr(struct PList *msgs, struct TypeExpr *type,
				  struct GlobExpr *e) {
	struct TypeExpr *char_type;

	if (!is_ptr_type(type)) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
		return;
	}

	char_type = &(struct TypeExpr){
		TC_PTR,
		{.ptr_target = &(struct TypeExpr){TC_UINT8, {.ptr_target = 0}}}};

	if (!are_types_equal(type, char_type)) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
	}
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

void cmpt_arr_ptr(struct PList *msgs, struct TypeExpr *type,
				  struct GlobExpr *e) {
	struct TypeExpr *array_type;
	struct GlobExpr *glob;
	uint32_t i;

	if (!is_ptr_type(type)) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
		return;
	}
	if (is_void_ptr(type)) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_CANT_DEFINE_ARR_TYPE);
		return;
	}

	array_type = ptr_targ(type);

	for (i = 0; i < e->globs->size; i++) {
		glob = plist_get(e->globs, i);
		are_types_compatible(msgs, array_type, glob);

		// FIXED: here too segafult possible
		if (glob->type)
			free_type(glob->type);
		glob->type = copy_type_expr(array_type);
	}
}

int arr_err_of_size(struct PList *msgs, struct GlobExpr *e, long size,
					long items) {
	if (size == -1)
		return 0;
	if (size < items) {
		plist_add(msgs, (void *)size);
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_TOO_MUCH_ITEMS_FOR_THIS_ARR);
		return TOO_MUCH_ITEMS;
	}
	if (size > items) {
		plist_add(msgs, (void *)size);
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_TOO_LESS_ITEMS_FOR_THIS_ARR);
		return NEED_ADD_ITEMS;
	}
	return 0;
}

void cmpt_arr(struct PList *msgs, struct TypeExpr *type, struct GlobExpr *e) {
	struct TypeExpr *array_type;
	struct GlobExpr *glob;
	long arr_size;
	uint32_t i;

	if (e->from) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_ARR_FROM_OTHER_GLOBAL_ARR);
		return;
	}
	if (is_ptr_type(type)) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_ARR_IS_NOT_A_PTR);
		return;
	}
	if (type->code != TC_ARR) {
		if (e->globs->size == 0) {
			// im lazy to do err, so just zero
			plist_add(e->globs,
					  new_zero_type(type, unsafe_size_of_type(type), e->tvar));
			return;
		}
		for (i = 0; i < e->globs->size; i++) {
			glob = plist_get(e->globs, i);
			if (glob->code == CT_ZERO)
				continue;

			are_types_compatible(msgs, type, glob);

			if (glob->type)
				free_type(glob->type);
			glob->type = copy_type_expr(type);
		}
		return;
	}

	array_type = arr_type(type);
	arr_size = (long)arr_len(type);

	if (arr_size == -1) { // need to adjust it by size of e->globs->size
		arr_size = e->globs->size;
		set_arr_len(type->data.arr, arr_size);
		goto check_items_of_the_arr_on_types;
	}
	if (arr_err_of_size(msgs, e, arr_size, e->globs->size) == NEED_ADD_ITEMS) {
		int item_size = unsafe_size_of_type(array_type);

		for (; e->globs->size < arr_size;)
			plist_add(e->globs, new_zero_type(array_type, item_size, e->tvar));
	}
check_items_of_the_arr_on_types:

	for (i = 0; i < e->globs->size; i++) {
		glob = plist_get(e->globs, i);
		if (glob->code == CT_ZERO)
			continue;

		are_types_compatible(msgs, array_type, glob);

		// FIXED by copy: here is for example [окак тип [...] [...] [...]]
		// it will segfault i beleive cuz all first arr types are same
		// as окак type потому что сначала происходит то что внутри
		// массивов а потом сами массивы изза рекурсии
		if (glob->type)
			free_type(glob->type);
		glob->type = copy_type_expr(array_type);
	}
}
