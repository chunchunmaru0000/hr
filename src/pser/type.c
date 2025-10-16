#include "pser.h"

const char *const ERR_WRONG_TOKEN_NUM_PAR_C_R =
	"Ожидалось целое число или скобка ']'.";
const char *const WRONG_ARR_SIZE =
	"Размера массива не может быть меньше -1, -1 значит любой размер.";
const char *const FUN_TYPE_END_OF_FILE =
	"Скобки типа функции не были закрыты и был достигнут конец файла.";
const char *const NOT_A_TYPE_WORD = "Ожидалось слово типа.";
const char *const FUN_ZERO_ARGS = "Тип функции не может иметь 0 аргументов.";
const char *const AMPER_WORKS_ONLY_ON_STRUCTS =
	"Знак '&' применим только к типу указателя на структуру, например: '&лик "
	"Чето'.";
const char *const STRUCT_NAME_WASNT_FOUND =
	"Имя лика не было найдено в уже объявленных.";
const char *const SUGGEST_ADD_ARGS = "добавить аргументов";

const struct TypeWord TYPE_WORDS[] = {
	{"ч8", TC_INT8, 3},	   {"ц8", TC_UINT8, 3},	  {"ч16", TC_INT16, 4},
	{"ц16", TC_UINT16, 4}, {"ч32", TC_INT32, 4},  {"ц32", TC_UINT32, 4},
	{"в32", TC_FLOAT, 4},  {"в64", TC_DOUBLE, 4}, {"ч64", TC_INT64, 4},
	{"ц64", TC_UINT64, 4}, {"тлен", TC_VOID, 8},
};

const struct TypeWord TYPE_WORD_STRUCT = {"лик", TC_STRUCT, 6};
const struct TypeWord TYPE_WORD_ENUM = {"счет", TC_ENUM, 8};
const struct TypeWord TYPE_WORD_PTR = {"ук", TC_PTR, 4};
const struct TypeWord TYPE_WORD_FUN = {"фц", TC_FUN, 4};

struct TypeExpr *copy_type_expr(struct TypeExpr *type) {
	if (type == 0)
		return 0;
	long i;
	struct TypeExpr *copy = new_type_expr(TC_VOID), *other;

	if (type->code < TC_PTR) {
		copy->code = type->code;
	} else if (type->code == TC_PTR) {
		// * if [[ ptr ]] -> TypeExpr *
		copy->data.ptr_target = copy_type_expr(type->data.ptr_target);
	} else if (type->code == TC_FUN) {
		// * if [[ fun ]] -> plist of TypeExpr * where last type is return type
		copy->data.args_types = new_plist(type->data.args_types->size);

		for (i = 0; i < type->data.args_types->size; i++) {
			other = plist_get(type->data.args_types, i);
			plist_add(copy->data.args_types, copy_type_expr(other));
		}
	} else if (type->code == TC_ARR) {
		// * if [[ arr ]] -> plist with two items
		copy->data.arr = new_plist(2);
		// * - first item is TypeExpr *
		other = arr_type(type);
		plist_set(copy->data.arr, 0, other);
		// * - second item is long that is len of arr, if len is -1 then any len
		set_arr_len(copy->data.arr, arr_len(type));
	} else if (type->code == TC_STRUCT) {
		// * if [[ struct ]] -> name blist
		copy->data.name = copy_str(type->data.name);
	} else
		exit(125);

	return copy;
}

struct BList *type_to_blist_from_str(struct TypeExpr *type);

void add_type_str_to_str(struct BList *str, struct TypeExpr *type) {
	struct BList *tmp = type_to_blist_from_str(type);
	copy_to_fst_and_clear_snd(str, tmp);
}

struct BList *type_to_blist_from_str(struct TypeExpr *type) {
	struct BList *str = new_blist(9), *tmp;
	const struct TypeWord *type_word;
	uint32_t i;
	long arr_length;

	if (type->code == TC_PTR) {
		blat(str, (uc *)TYPE_WORD_PTR.view, TYPE_WORD_PTR.view_len);
		blist_add(str, '_');
		add_type_str_to_str(str, ptr_targ(type));
	} else if (type->code == TC_STRUCT) {
		blat(str, (uc *)TYPE_WORD_STRUCT.view, TYPE_WORD_STRUCT.view_len);
		blist_add(str, '_');
		blat_blist(str, type->data.name);
	} else if (type->code == TC_ENUM) {
		blat(str, (uc *)TYPE_WORD_ENUM.view, TYPE_WORD_ENUM.view_len);
		blist_add(str, '_');
		blat_blist(str, type->data.name);
	} else if (type->code == TC_ARR) {
		blist_add(str, '[');

		add_type_str_to_str(str, arr_type(type));
		blist_add(str, '_');

		arr_length = (long)arr_len(type);
		if (arr_length == -1)
			blist_add(str, '~');
		else {
			tmp = int_to_str(arr_length);
			copy_to_fst_and_clear_snd(str, tmp);
		}

		blist_add(str, ']');
	} else if (type->code == TC_FUN) {
		// blat(str, (uc *)TYPE_WORD_FUN.view, TYPE_WORD_FUN.view_len);
		blist_add(str, '{');
		// -1 cuz lasr itteration after it
		for (i = 0; i < type->data.args_types->size - 1; i++) {
			add_type_str_to_str(str, plist_get(type->data.args_types, i));
			blist_add(str, '_');
		}
		// this is last itteration, i just dont wanna do if in the loop above
		add_type_str_to_str(str, plist_get(type->data.args_types, i));

		blist_add(str, '}');
	} else {
		for (i = 0; i < loa(TYPE_WORDS); i++) {
			type_word = TYPE_WORDS + i;

			if (type_word->code == type->code) {
				blat(str, (uc *)type_word->view, type_word->view_len);
				break;
			}
		}
	}

	// need to do convert_blist_to_blist_from_str after
	// like in get_global_signature
	return str;
}

void get_fun_signature_considering_args(struct PList *os, struct GlobVar *var) {
	struct BList *signature = new_blist(32);
	struct Arg *arg, *next_arg;
	uint32_t i;

	// add name
	blat_blist(signature, var->name->view);
	blist_add(signature, '_');
	blist_add(signature, '_');
	// start fun type part
	blist_add(signature, '{');

	next_arg = plist_get(os, 1);
	if (!next_arg) // zero term
		goto skip_add_args;

	for (i = 1; next_arg;) {
		arg = next_arg;

		add_type_str_to_str(signature, arg->type);

		next_arg = plist_get(os, ++i);
		if (!next_arg) // zero term
			break;

		if (arg->offset == next_arg->offset)
			blist_add(signature, ',');
		else
			blist_add(signature, '_');
	}
	blist_add(signature, '_');

skip_add_args:

	// last arg type from var->type that is return type
	add_type_str_to_str(signature,
						plist_get(var->type->data.args_types,
								  var->type->data.args_types->size - 1));
	// end fun type part
	blist_add(signature, '}');

	convert_blist_to_blist_from_str(signature);
	var->signature = signature;
}

void get_global_signature(struct GlobVar *var) {
	var->signature = new_blist(128);
	blat_blist(var->signature, var->name->view);
	blist_add(var->signature, '_');
	blist_add(var->signature, '_');
	add_type_str_to_str(var->signature, var->type);

	convert_blist_to_blist_from_str(var->signature);
}

int are_types_equal(struct TypeExpr *t1, struct TypeExpr *t2) {
	if (t1->code != t2->code) {
		if (is_void_ptr(t1) && is_ptr_type(t2))
			return 1; // void ptr and any ptr
		if (is_ptr_type(t1) && is_void_ptr(t2))
			return 1; // any ptr and void ptr
		if (t1->code == TC_ARR)
			return are_types_equal(arr_type(t1), t2);
		if (t2->code == TC_ARR)
			return are_types_equal(arr_type(t2), t1);

		return 0;
	}

	// here codes are equal so code of t1 is aslo code of t2
	if (t1->code < TC_PTR)
		return 1;

	uint32_t res = 0;
	long size1, size2;

	if (t1->code == TC_PTR) {
		if (is_void_ptr(t1) || is_void_ptr(t2))
			return 1; // cuz void ptr are equal to any ptr
		res = are_types_equal(ptr_targ(t1), ptr_targ(t2));
	} else if (t1->code == TC_STRUCT || t1->code == TC_ENUM) {
		res = sc((char *)t1->data.name->st, (char *)t2->data.name->st);
	} else if (t1->code == TC_ARR) {
		size1 = (long)arr_len(t1);
		size2 = (long)arr_len(t2);
		res = ((size1 == -1) || (size2 == -1) || (size1 == size2));
		if (res == 0)
			return 0;
		res = res && are_types_equal(arr_type(t1), arr_type(t2));
	} else { // TC_FUN
		if (t1->data.args_types->size != t2->data.args_types->size)
			return 0;
		for (; res < t1->data.args_types->size; res++) {
			if (!are_types_equal(plist_get(t1->data.args_types, res),
								 plist_get(t2->data.args_types, res)))
				return 0;
		}
		res = 1;
	}

	return res;
}

void free_type(struct TypeExpr *type) {
	uint32_t i;

	if (type->code == TC_PTR)
		free_type(ptr_targ(type));

	else if (type->code == TC_ARR) {
		free_type(arr_type(type));
		plist_free(type->data.arr);

	} else if (type->code == TC_FUN) {
		for (i = 0; i < type->data.args_types->size; i++)
			free_type(plist_get(type->data.args_types, i));
		plist_free(type->data.args_types);
	}
	// else if (type->code == TC_STRUCT || type->code == TC_ENUM)
	//     ; // nothing cuz this blist is part of a token

	free(type);
}

const char *const STR_STR_TW = "стр";
const char *const STR_STRUCT_TW = "лик";
const char *const STR_ENUM_TW = "счет";

struct TypeExpr *new_type_expr(enum TypeCode code) {
	struct TypeExpr *texpr = malloc(sizeof(struct TypeExpr));
	texpr->data.ptr_target = 0;
	texpr->code = code;
	return texpr;
}

struct TypeExpr *type_expr(struct Pser *p) {
	const struct TypeWord *tw;
	struct Token *cur = pser_cur(p);
	struct TypeExpr *texpr = new_type_expr(TC_VOID);

	if (cur->code == ID) {
		if (vcs(cur, STR_STR_TW)) {
			texpr->code = TC_PTR;
			texpr->data.ptr_target = new_type_expr(TC_UINT8);

		} else if (1) {
			if (vcs(cur, STR_STRUCT_TW)) {
				texpr->code = TC_PTR;
				texpr->data.ptr_target = new_type_expr(TC_STRUCT);

				cur = absorb(p);
				expect(cur, ID);

				texpr->data.ptr_target->data.name = cur->view;
			} else if (vcs(cur, STR_ENUM_TW)) {
				texpr->code = TC_ENUM;

				cur = absorb(p);
				expect(cur, ID);

				texpr->data.name = cur->view;
			} else
				goto check_type_words;

		} else {
		check_type_words:
			for (size_t i = 0; i < loa(TYPE_WORDS); i++) {
				tw = TYPE_WORDS + i;

				if (vcs(cur, tw->view)) {
					texpr->code = tw->code;
					goto ret_type_expr;
				}
			}
			eet(cur, NOT_A_TYPE_WORD, 0);
		}
	} else if (cur->code == MUL) {
		texpr->code = TC_PTR;
		consume(p); // consume *
		texpr->data.ptr_target = type_expr(p);
		return texpr; // no need to consume

	} else if (cur->code == AMPER) {
		consume(p); // consume &
		struct TypeExpr *strct = type_expr(p);

		if (strct->code != TC_PTR || strct->data.ptr_target->code != TC_STRUCT)
			eet(cur, AMPER_WORKS_ONLY_ON_STRUCTS, 0);

		free(texpr);
		texpr = strct->data.ptr_target;
		free(strct);

		return texpr; // no need to consume
	} else if (cur->code == PAR_C_L) {
		texpr->code = TC_ARR;
		consume(p);

		texpr->data.arr = new_plist(2);
		plist_add(texpr->data.arr, type_expr(p));

		cur = pser_cur(p);
		if (cur->code == PAR_C_R)
			plist_add(texpr->data.arr, (void *)-1);
		else if (cur->code == INT) {
			if (cur->num < -1)
				eet(cur, WRONG_ARR_SIZE, 0);
			plist_add(texpr->data.arr, (void *)cur->num);
			consume(p); // consume arr size
			cur = pser_cur(p);
		} else
			eet(cur, ERR_WRONG_TOKEN_NUM_PAR_C_R, 0);

		expect(cur, PAR_C_R); // expect ]

	} else if (cur->code == PAR_L) {
		texpr->code = TC_FUN;
		texpr->data.args_types = new_plist(2);

		cur = absorb(p);
		while (cur->code != PAR_R && cur->code != EXCL && cur->code != EF) {
			plist_add(texpr->data.args_types, type_expr(p));
			cur = pser_cur(p);
		}
		if (cur->code == EXCL) {
			consume(p); // consume !
			plist_add(texpr->data.args_types, type_expr(p));

			expect(pser_cur(p), PAR_R); // expect )

		} else if (cur->code == PAR_R) {
			if (texpr->data.args_types->size == 0)
				eet(cur, FUN_ZERO_ARGS, SUGGEST_ADD_ARGS);
		} else
			eet(cur, FUN_TYPE_END_OF_FILE, 0);
	} else
		eet(cur, NOT_A_TYPE_WORD, 0);

ret_type_expr:
	consume(p); // ] ) type_word struct_name str
	return texpr;
}
