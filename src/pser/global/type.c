#include "../pser.h"
#include <stdio.h>

constr NUM_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с числовым типом выражения.";
constr STR_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с строковым типом выражения.";
constr ARR_SIZES_DO_NOW_MATCH =
	"Рамеры типа переменной массива и самого массива не совпадают.";
constr PTR_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с указываемым типом выражения.";
constr FUN_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с функциональным типом выражения.";
constr ARR_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с типом выражения массива.";
constr STRUCT_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с типом выражения лика.";
constr ARR_ITEM_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с типом выражения значения массива.";
constr ARR_FROM_OTHER_GLOBAL_ARR =
	"Нелязя назначать массив от другого массива через его имя, только если "
	"через указатель.";
constr STRUCT_FROM_OTHER_GLOBAL_STRUCT =
	"Нелязя назначать лик от другого лик через его имя, только если "
	"через указатель.";
constr AS_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с приведенным типом выражения.";
constr STRUCT_WASNT_FOUND = "Лик с таким именем не был найден.";
constr INCOMPATIBLE_TYPES = "Типы переменной и выражения несовместимы.";
constr TOO_MUCH_FIELDS_FOR_THIS_STRUCT =
	"Слишком много аргументов указано для данной структуры.";
constr TOO_LESS_FIELDS_FOR_THIS_STRUCT =
	"Слишком мало аргументов указано для данной структуры, остальная ее часть "
	"будет заполнена нулями.";
constr TOO_MUCH_ITEMS_FOR_THIS_ARR =
	"Слишком много элементов для массива данного типа.";
constr TOO_LESS_ITEMS_FOR_THIS_ARR =
	"Слишком мало элементов для массива данного типа.";
constr TOO_MUCH_CHARS_FOR_THIS_STR = "Слишком много байт для данной строки.";
constr TOO_LESS_CHARS_FOR_THIS_STR = "Слишком мало байт для данной строки.";
constr EXCESSING_FIELD =
	"Лишнее поле в лике, его место в памяти уже "
	"возможно занято или в данном лике вообще нет аргумента с таким именем.";
constr UNCOMPUTIBLE_DATA = "Невычислимое выражение.";
constr EXPECTED_ARR_OF_LEN = "ожидался массив длиной: ";
constr EXPECTED_STRUCT_OF_LEN = "ожидалось аргументов: ";
constr STR_IS_NOT_A_PTR =
	"Строка - не указатель, строка - массив, массив - не указатель, "
	"но строковые литералы по умолчанию возвращают указатели на себя, чтобы "
	"получить из них массив надо разыменовать строку вручную, "
	"например:\n\tпусть с: [ц8] = *\"строка\"";
constr ARR_IS_NOT_A_PTR =
	"Массив - не указатель, массив - несколько значений одного типа, чтобы "
	"получить указатель из массива, надо взять его адрес, например:\n\tпусть "
	"м: *ч64 = &[1 2 3]";
constr CANT_DEFINE_ARR_TYPE =
	"Тип недостаточно явный для определения типа массива.";
constr CANT_DEFINE_STRUCT_TYPE =
	"Тип недостаточно явный для определения типа лика.";
constr MAYBE_NOT_USE_VOID_PTR = "может не использовать указатель на тлен";
constr todo_str =
	"need to do in pser_tpes.c aat the end of the are_types_compatible";

struct CE_CodeStr {
	enum CE_Code code;
	constr str;
	constr sgst;
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
	{CE_TOO_MUCH_ITEMS_FOR_THIS_ARR, TOO_MUCH_ITEMS_FOR_THIS_ARR,
	 EXPECTED_ARR_OF_LEN},
	{CE_TOO_MUCH_CHARS_FOR_THIS_STR, TOO_MUCH_CHARS_FOR_THIS_STR,
	 EXPECTED_ARR_OF_LEN},
	{CE_STR_IS_NOT_A_PTR, STR_IS_NOT_A_PTR, 0},
	{CE_ARR_IS_NOT_A_PTR, ARR_IS_NOT_A_PTR, 0},
	{CE_UNCOMPUTIBLE_DATA, UNCOMPUTIBLE_DATA, 0},
	{CE_EXCESSING_FIELD, EXCESSING_FIELD, 0},
	{CE_CANT_DEFINE_ARR_TYPE, CANT_DEFINE_ARR_TYPE, MAYBE_NOT_USE_VOID_PTR},
	{CE_CANT_DEFINE_STRUCT_TYPE, CANT_DEFINE_STRUCT_TYPE,
	 MAYBE_NOT_USE_VOID_PTR},

	{CE_todo, todo_str, 0},
};
const struct CE_CodeStr cecstrs_warns[] = {
	{CE_ARR_SIZES_DO_NOW_MATCH, ARR_SIZES_DO_NOW_MATCH, EXPECTED_ARR_OF_LEN},
	{CE_TOO_LESS_FIELDS_FOR_THIS_STRUCT, TOO_LESS_FIELDS_FOR_THIS_STRUCT,
	 EXPECTED_STRUCT_OF_LEN},
	{CE_TOO_LESS_ITEMS_FOR_THIS_ARR, TOO_LESS_ITEMS_FOR_THIS_ARR,
	 EXPECTED_ARR_OF_LEN},
	{CE_TOO_LESS_CHARS_FOR_THIS_STR, TOO_LESS_CHARS_FOR_THIS_STR,
	 EXPECTED_ARR_OF_LEN},
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
			ei = new_error_info(err_token, cstr->str, cstr->sgst);

			if (cstr->code == CE_TOO_MUCH_FIELDS_FOR_THIS_STRUCT ||
				cstr->code == CE_TOO_MUCH_ITEMS_FOR_THIS_ARR ||
				cstr->code == CE_TOO_MUCH_CHARS_FOR_THIS_STR) {

				ei->extra = (void *)plist_get(msgs, --i);
				ei->extra_type = ET_INT;
			}

			plist_add(p->errors, ei);
			continue;
		}

		if ((cstr = find_warn_msg(error))) {
			err_token = plist_get(msgs, --i);
			ei = new_error_info(err_token, cstr->str, cstr->sgst);

			ei->extra = (void *)plist_get(msgs, --i);
			if (cstr->code == CE_ARR_SIZES_DO_NOW_MATCH ||
				cstr->code == CE_TOO_LESS_FIELDS_FOR_THIS_STRUCT ||
				cstr->code == CE_TOO_LESS_CHARS_FOR_THIS_STR ||
				cstr->code == CE_TOO_LESS_ITEMS_FOR_THIS_ARR)

				ei->extra_type = ET_INT;

			plist_add(p->warns, ei);
			continue;
		}
	}
}

void (*cmpts[])(struct PList *msgs, struct TypeExpr *type,
				struct GlobExpr *e) = {
	cmpt_int,		 // 	CT_INT
	cmpt_real,		 // 	CT_REAL
	cmpt_fun,		 // 	CT_FUN
	cmpt_str,		 // 	CT_STR
	cmpt_str_ptr,	 // 	CT_STR_PTR
	cmpt_arr,		 // 	CT_ARR
	cmpt_arr_ptr,	 // 	CT_ARR_PTR
	cmpt_struct,	 // 	CT_STRUCT
	cmpt_struct_ptr, // 	CT_STRUCT_PTR
	cmpt_global,	 // 	CT_GLOBAL
	cmpt_global_ptr, // 	CT_GLOBAL_PTR
					 // 	CT_ZERO
};

void are_types_compatible(struct PList *msgs, struct TypeExpr *type,
						  struct GlobExpr *e) {
	long arr_size;

	if (e->type) {
		if (type->code == TC_ARR && e->type->code != TC_ARR) {
			type = arr_type(type);
		}
		// compares global types, not returns enum CE_Code, just boolean
		if (!are_types_equal(type, e->type) &&
			unsafe_size_of_type(e->type) != unsafe_size_of_type(type)) {
			plist_add(msgs, e->tvar);
			plist_add(msgs, (void *)CE_AS_INCOMPATIBLE_TYPE);
			return;
		}
		return;
	}

	// 	// and e here is not CT_GLOBAL
	// 	if (type->code == TC_ARR && e->code != CT_ARR &&
	// !is_compile_time_ptr(e))
	if (type->code == TC_ARR && e->code != CT_ARR && e->code != CT_STR) {
		arr_size = (long)arr_len(type);

		arr_err_of_size(msgs, e, arr_size, 1);
		are_types_compatible(msgs, arr_type(type), e);
		set_arr_len(type->data.arr, 1); // 1 element
		return;
	}
	if (type->code == TC_PTR && e->code == CT_INT && e->tvar->num == 0)
		return;

	if (e->code != CT_ZERO) {
		cmpts[e->code](msgs, type, e);
		return;
	}

	plist_add(msgs, e->tvar);
	plist_add(msgs, (void *)CE_todo);
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
