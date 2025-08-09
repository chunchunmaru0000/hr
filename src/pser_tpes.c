#include "pser.h"

enum CE_Code {
	CE_NONE,
	CE_NUM_INCOMPATIBLE_TYPE,
	CE_STR_INCOMPATIBLE_TYPE,
	CE_ARR_SIZES_DO_NOW_MATCH,
	CE_PTR_INCOMPATIBLE_TYPE,
	CE_FUN_INCOMPATIBLE_TYPE,
	CE_ARR_INCOMPATIBLE_TYPE,
	CE_STRUCT_INCOMPATIBLE_TYPE,
	CE_AS_INCOMPATIBLE_TYPE,
	CE_ARR_ITEM_INCOMPATIBLE_TYPE,
	CE_ARR_FROM_OTHER_GLOBAL_ARR,
	CE_STRUCT_FROM_OTHER_GLOBAL_STRUCT,
	CE_TOO_MUCH_FIELDS_FOR_THIS_STRUCT,
	CE_TOO_LESS_FIELDS_FOR_THIS_STRUCT,
	CE_STRUCT_WASNT_FOUND,
	CE_STR_IS_NOT_A_PTR,
	CE_ARR_IS_NOT_A_PTR,
	CE_UNCOMPUTIBLE_DATA,

	CE_TODO1,
	CE_TODO3,
	CE_TODO4,
};

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
const char *const STRUCT_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с типом выражения лика.";
const char *const ARR_ITEM_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с типом выражения значения массива.";
const char *const ARR_FROM_OTHER_GLOBAL_ARR =
	"Нелязя назначать массив от другого массива через его имя, только если "
	"через указатель.";
const char *const STRUCT_FROM_OTHER_GLOBAL_STRUCT =
	"Нелязя назначать лик от другого лик через его имя, только если "
	"через указатель.";
const char *const AS_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с приведенным типом выражения.";
const char *const STRUCT_WASNT_FOUND = "Лик с таким именем не был найден.";
const char *const INCOMPATIBLE_TYPES =
	"Типы переменной и выражения несовместимы.";
const char *const TOO_MUCH_FIELDS_FOR_THIS_STRUCT =
	"Слишком много аргументов указано для данной структуры.";
const char *const TOO_LESS_FIELDS_FOR_THIS_STRUCT =
	"Слишком мало аргументов указано для данной структуры, остальная ее часть "
	"будет заполнена нулями.";
const char *const UNCOMPUTIBLE_DATA = "Невычислимое выражение.";
const char *const EXPECTED_ARR_OF_LEN = "ожидался массив длиной: ";
const char *const EXPECTED_STRUCT_OF_LEN = "ожидалось аргументов: ";
const char *const STR_IS_NOT_A_PTR =
	"Строка - не указатель, строка - массив, массив - не указатель, чтобы "
	"получить указатель из строки нужно взять ее адрес, например:\n\tпусть с: "
	"стр = &\"строка\"";
const char *const ARR_IS_NOT_A_PTR =
	"Массив - не указатель, массив - несколько значений одного типа, чтобы "
	"получить указатель из массива, надо взять его адрес, например:\n\tпусть "
	"м: *ч64 = &[1 2 3]";

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
	{CE_STRUCT_INCOMPATIBLE_TYPE, STRUCT_INCOMPATIBLE_TYPE, 0},
	{CE_FUN_INCOMPATIBLE_TYPE, FUN_INCOMPATIBLE_TYPE, 0},
	{CE_AS_INCOMPATIBLE_TYPE, AS_INCOMPATIBLE_TYPE, 0},
	{CE_ARR_ITEM_INCOMPATIBLE_TYPE, ARR_ITEM_INCOMPATIBLE_TYPE, 0},
	{CE_ARR_FROM_OTHER_GLOBAL_ARR, ARR_FROM_OTHER_GLOBAL_ARR, 0},
	{CE_STRUCT_FROM_OTHER_GLOBAL_STRUCT, STRUCT_FROM_OTHER_GLOBAL_STRUCT, 0},
	{CE_TOO_MUCH_FIELDS_FOR_THIS_STRUCT, TOO_MUCH_FIELDS_FOR_THIS_STRUCT,
	 EXPECTED_STRUCT_OF_LEN},
	{CE_STRUCT_WASNT_FOUND, STRUCT_WASNT_FOUND, 0},
	{CE_STR_IS_NOT_A_PTR, STR_IS_NOT_A_PTR, 0},
	{CE_ARR_IS_NOT_A_PTR, ARR_IS_NOT_A_PTR, 0},
	{CE_UNCOMPUTIBLE_DATA, UNCOMPUTIBLE_DATA, 0},

	{CE_TODO1, "TODO1", 0},
	{CE_TODO3, "TODO3", 0},
	{CE_TODO4, "TODO4", 0},
};
const struct CE_CodeStr cecstrs_warns[] = {
	{CE_ARR_SIZES_DO_NOW_MATCH, ARR_SIZES_DO_NOW_MATCH, EXPECTED_ARR_OF_LEN},
	{CE_TOO_LESS_FIELDS_FOR_THIS_STRUCT, TOO_LESS_FIELDS_FOR_THIS_STRUCT,
	 EXPECTED_STRUCT_OF_LEN},
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

			if (cstr->code == CE_TOO_MUCH_FIELDS_FOR_THIS_STRUCT) {
				ei->extra = (void *)plist_get(msgs, --i);
				ei->extra_type = ET_INT;
			}

			plist_add(p->errors, ei);
			continue;
		}

		if ((cstr = find_warn_msg(error))) {
			err_token = plist_get(msgs, --i);
			ei = new_error_info(p->f, err_token, cstr->str, cstr->sgst);

			ei->extra = (void *)plist_get(msgs, --i);
			if (cstr->code == CE_ARR_SIZES_DO_NOW_MATCH)
				ei->extra_type = ET_INT;
			else if (cstr->code == CE_TOO_LESS_FIELDS_FOR_THIS_STRUCT)
				ei->extra_type = ET_INT;

			plist_add(p->warns, ei);
			continue;
		}
	}
}

struct GlobExpr *new_zero_type(struct Arg *arg, struct Token *tvar) {
	struct GlobExpr *e = malloc(sizeof(struct GlobExpr));

	e->code = CT_ZERO;
	e->from = 0;
	e->type = arg->type;

	// not used for defining value, just in case here
	e->tvar = malloc(sizeof(struct Token));
	e->tvar->view = tvar->view;
	e->tvar->number = arg->arg_size;

	e->globs = 0;

	return e;
}

// TODO: if e->globs->size is less then arr_size(type) then just warn else err
// cur arr size if defined then it its size for it and it wont change later cuz
// cant do it and need to fill it with zeros
void are_types_compatible(struct PList *msgs, struct TypeExpr *type,
						  struct GlobExpr *e) {
	long n;
	struct TypeExpr *tmp_type;
	struct GlobExpr *glob;
	enum CE_Code tmp_code = CE_NONE;

	if (e->type) {
		// compares global types, not returns enum CE_Code, just boolean
		if (!are_types_equal(type, e->type)) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_AS_INCOMPATIBLE_TYPE);
			return;
		}

		if (type->code == TC_ARR && e->type->code != TC_ARR) {
			// TODO check if arr and its value
		}
		return;
	}

	if (e->code == CT_GLOBAL) {
		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)CE_UNCOMPUTIBLE_DATA);
		return;
	}

	// 	// and e here is not CT_GLOBAL
	// 	if (type->code == TC_ARR && e->code != CT_ARR &&
	// !is_compile_time_ptr(e)) { 		n = msgs->size;
	//
	// 		are_types_compatible(msgs, arr_type(type), e);
	//
	// 		// TODO: it better after cuz if warn will also err
	// 		if (n == msgs->size) {
	// 			// need to do arr size = 1
	// 			n = (long)arr_size(type);
	// 			if (n != -1 && n != 1) {
	// 				plist_add(msgs, (void *)n);
	// 				plist_add(msgs, e->tvar);
	// 				plist_add(msgs, (void *)CE_ARR_SIZES_DO_NOW_MATCH);
	// 			}
	//
	// 			plist_set(type->data.arr, 1, (void *)1); // size of 1
	// 			return;
	// 		}
	// 	}

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
		tmp_type = arr_type(type);
		if (tmp_type->code != TC_UINT8) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_STR_INCOMPATIBLE_TYPE);
			return;
		}

		// asume array size
		n = (long)arr_size(type);
		if (n != -1 && n != e->tvar->str->size + 1) {
			plist_add(msgs, (void *)n);
			tmp_code = CE_ARR_SIZES_DO_NOW_MATCH;
		}
		// set size in any way
		n = e->tvar->str->size + 1; // + 1 cuz '\0'
		plist_set(type->data.arr, 1, (void *)n);

		plist_add(msgs, e->tvar);
		plist_add(msgs, (void *)tmp_code);
		return;
	}

	if (e->code == CT_ARR) {
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
		// TODO: check if e->code is compatible with arr_type(type)
		if (type->code != TC_ARR) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_ARR_INCOMPATIBLE_TYPE);
			return;
		}

		tmp_type = arr_type(type);

		for (n = 0; n < e->globs->size; n++) {
			glob = plist_get(e->globs, n);
			are_types_compatible(msgs, tmp_type, glob);

			// TODO: here is for example [окак тип [...] [...] [...]]
			// it will segfault i beleive cuz all first arr types are same
			// as окак type потому что сначала происходит то что внутри
			// массивов а потом сами массивы изза рекурсии
			if (glob->type)
				free_type(glob->type);
			glob->type = tmp_type;
		}

		n = (long)arr_size(type);
		if (n != -1 && n != e->globs->size) {
			plist_add(msgs, (void *)n);
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_ARR_SIZES_DO_NOW_MATCH);
		}

		// TODO: here need to do if size was already once changed then err
		// set size in any way
		n = e->globs->size;
		plist_set(type->data.arr, 1, (void *)n);

		return;
	}

	if (e->code == CT_STRUCT) {
		if (e->from) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_STRUCT_FROM_OTHER_GLOBAL_STRUCT);
			return;
		}
		if (type->code != TC_STRUCT) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_STRUCT_INCOMPATIBLE_TYPE);
			return;
		}

		struct PList *lik_os = find_lik_os(type->data.name);
		if (lik_os == 0) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_STRUCT_WASNT_FOUND);
			return;
		}
		long lik_mems = (long)plist_get(lik_os, DCLR_STRUCT_MEMS);

		if (e->globs->size > lik_mems) {
			plist_add(msgs, (void *)lik_mems);
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_TOO_MUCH_FIELDS_FOR_THIS_STRUCT);
			return;
		}
		if (lik_mems > e->globs->size) {
			plist_add(msgs, (void *)lik_mems);
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_TOO_LESS_FIELDS_FOR_THIS_STRUCT);

			struct Arg *arg;
			// get first arg
			arg = get_arg_by_mem_index(lik_os, e->globs->size);
			plist_add(e->globs, new_zero_type(arg, e->tvar));
			// get all other args
			for (; e->globs->size < lik_mems;) {
				arg = get_arg_of_next_offset(lik_os, arg->offset);
				plist_add(e->globs, new_zero_type(arg, e->tvar));
			}
		}

		for (n = 0; n < e->globs->size; n++) {
			glob = plist_get(e->globs, n);

			if (glob->code == CT_ZERO)
				continue;

			are_types_compatible(msgs, tmp_type, glob);
			if (glob->type)
				free_type(glob->type);
			glob->type = tmp_type;
		}
		return;
	}

	// here e->from != 0
	if (e->code == CT_GLOBAL_PTR) {
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
		tmp_type = &(struct TypeExpr){TC_PTR, {.ptr_target = e->from->type}};

		// TODO: if types sizes arent equal, need to i dunno flag if it matters
		// or kinda thing, its easer then have it return an enum
		if (!are_types_equal(type, tmp_type)) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
		}
		return;
	}

	if (e->code == CT_STR_PTR) {
		if (!is_ptr_type(type)) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
			return;
		}

		tmp_type = &(struct TypeExpr){
			TC_PTR,
			{.ptr_target = &(struct TypeExpr){TC_UINT8, {.ptr_target = 0}}}};

		if (!are_types_equal(type, tmp_type)) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
		}
		return;
	}

	if (e->code == CT_ARR_PTR) {
		if (!is_ptr_type(type)) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_PTR_INCOMPATIBLE_TYPE);
			return;
		}

		tmp_type = ptr_targ(type);

		for (n = 0; n < e->globs->size; n++) {
			glob = plist_get(e->globs, n);
			are_types_compatible(msgs, tmp_type, glob);

			// TODO: here too segafult possible
			if (glob->type)
				free_type(glob->type);
			glob->type = tmp_type;
		}
		return;
	}

	if (e->code == CT_STRUCT_PTR) {
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

void check_global_type_compatibility(struct Pser *p, struct TypeExpr *type,
									 struct GlobExpr *e) {
	struct PList *msgs = new_plist(2);

	are_types_compatible(msgs, type, e);
	if (msgs->size != 0) {
		search_error_code(p, msgs);
		if (pser_need_err(p))
			pser_err(p);
	}

	plist_free(msgs);
}
