#include "pser.h"

const char *const NUM_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с числовым типом выражения.";
const char *const STR_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с строковым типом выражения.";
const char *const ARR_SIZES_DO_NOW_MATCH =
	"Рамеры типа переменной массива и самого массива не совпадают.";
const char *const PTR_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с указываемым типом выражения.";
const char *const FUN_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с функциональным типом выражения.";
const char *const ARR_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с типом выражения массива.";
const char *const ARR_ITEM_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с типом выражения значения массива.";
const char *const INCOMPATIBLE_TYPES =
	"Типы переменной и выражения несовместимы.";
const char *const UNCOMPUTIBLE_DATA = "Невычислимое выражение.";
const char *const ARR_LEN_WAS = "длина была ";

struct CE_CodeStr {
	enum CE_Code code;
	const char *const str;
	const char *const sgst;
};

const struct CE_CodeStr cecstrs_errs[] = {
	{CE_NUM_INCOMPATIBLE_TYPE, NUM_INCOMPATIBLE_TYPE, 0},
	{CE_STR_INCOMPATIBLE_TYPE, STR_INCOMPATIBLE_TYPE, 0},
	{CE_PTR_INCOMPATIBLE_TYPE, PTR_INCOMPATIBLE_TYPE, 0},
	{CE_ARR_INCOMPATIBLE_TYPE, ARR_INCOMPATIBLE_TYPE, 0},
	{CE_FUN_INCOMPATIBLE_TYPE, FUN_INCOMPATIBLE_TYPE, 0},
	{CE_ARR_ITEM_INCOMPATIBLE_TYPE, ARR_ITEM_INCOMPATIBLE_TYPE, 0},
	{CE_UNCOMPUTIBLE_DATA, UNCOMPUTIBLE_DATA, 0},

	{CE_TODO1, "TODO1", 0},
	{CE_TODO2, "TODO2", 0},
	{CE_TODO3, "TODO3", 0},
	{CE_TODO4, "TODO4", 0},
};
const struct CE_CodeStr cecstrs_warns[] = {
	{CE_ARR_SIZES_DO_NOW_MATCH, ARR_SIZES_DO_NOW_MATCH, ARR_LEN_WAS},
};

const struct CE_CodeStr *find_error_msg(enum CE_Code err_code) {
	const struct CE_CodeStr *cstr;
	uint32_t i;

	for (i = 0; i < loa(cecstrs_errs); i++) {
		cstr = cecstrs_errs + i;
		if (err_code == cstr->code)
			return cstr;
	}
	return 0;
}
const struct CE_CodeStr *find_warn_msg(enum CE_Code err_code) {
	const struct CE_CodeStr *cstr;
	uint32_t i;

	for (i = 0; i < loa(cecstrs_warns); i++) {
		cstr = cecstrs_warns + i;
		if (err_code == cstr->code)
			return cstr;
	}
	return 0;
}

void search_error_code(struct Pser *p, struct PList *msgs) {
	struct ErrorInfo *ei;

	const struct CE_CodeStr *cstr;
	struct Token *err_token;
	long error;
	int i;

	for (i = msgs->size - 1; i >= 0; i--) {
		error = (long)plist_get(msgs, i);

		if (error == CE_NONE) {
			i--;
			continue;
		}

		if ((cstr = find_error_msg(error))) {
			err_token = plist_get(msgs, --i);
			ei = new_error_info(p->f, err_token, cstr->str, cstr->sgst);
			plist_add(p->errors, ei);
			continue;
		}

		if ((cstr = find_warn_msg(error))) {
			err_token = plist_get(msgs, --i);
			ei = new_error_info(p->f, err_token, cstr->str, cstr->sgst);

			ei->extra = (void *)plist_get(msgs, --i);
			if (cstr->code == CE_ARR_SIZES_DO_NOW_MATCH)
				ei->extra_type = ET_INT;

			plist_add(p->warns, ei);
			continue;
		}
	}
}

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
		if (n != -1 && n != e->tvar->str->size) {
			n = e->tvar->str->size;
			plist_add(msgs, (void *)n);
			tmp_code = CE_ARR_SIZES_DO_NOW_MATCH;
		}
		// set size in any way
		n = e->tvar->view->size - 2;
		plist_set(type->data.arr, 1, (void *)n);

		plist_add(msgs, e->tvar);
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
				are_types_compatible(msgs, arr_type(type),
									 plist_get(e->globs, n));
			}

			n = (long)arr_size(type);
			if (n != -1 && n != e->globs->size) {
				n = e->globs->size;
				plist_add(msgs, (void *)n);
				plist_add(msgs, e->tvar);
				plist_add(msgs, (void *)CE_ARR_SIZES_DO_NOW_MATCH);
			}

			// set size in any way
			n = e->globs->size;
			plist_set(type->data.arr, 1, (void *)n);
		} else {
			for (n = 0; n < e->globs->size; n++) {
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
