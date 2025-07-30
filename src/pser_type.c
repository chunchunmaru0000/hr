#include "pser.h"

const char *const ERR_WRONG_TOKEN_NUM_PAR_C_R =
	"Ожидалось целое число или скобка ']'.";
const char *const WRONG_ARR_SIZE =
	"Размера массива не может быть меньше -1, -1 значит любой размер.";
const char *const FUN_TYPE_END_OF_FILE =
	"Скобки типа функции не были закрыты и был достигнут конец файла.";
const char *const NOT_A_TYPE_WORD = "Ожидалось слово типа.";
const char *const FUN_ZERO_ARGS = "Тип функции не может иметь 0 аргументов.";
const char *const SUGGEST_ADD_ARGS = "добавить аргументов";

int get_type_code_size(enum TypeCode code) {
	return code >= TC_VOID	  ? QWORD
		   : code >= TC_INT32 ? DWORD
		   : code >= TC_INT16 ? WORD
							  : BYTE;
}

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

struct BList *int_to_hex_str(long num) {
	char *num_view = malloc(11); // 0x23456789 = 10 chars + 0 term
	int four_bits, i;
	num_view[10] = 0;
	num_view[0] = '0';
	num_view[1] = 'x';

	for (i = 0; i < 8; i++) {
		four_bits = ((num >> ((7 - i) * 4)) & 0b1111);
		num_view[2 + i] = four_bits + (four_bits < 0xa ? '0' : 'a' - 0xa);
	}

	return blist_from_str(num_view, 10);
}

struct BList *int_to_str(long num) {
	char *num_view;
	long num_clone;
	int num_len = 0, minus_flag = 0;

	if (!num) {
		num_view = malloc(2);
		num_view[1] = '\0';
		num_view[0] = '0';
		return blist_from_str(num_view, 1);
	}

	if (num < 0) {
		minus_flag = 1;
		num_len = 1;
		num *= -1;
	}
	num_clone = num;

	for (; num_clone; num_len++)
		num_clone /= 10;

	num_view = malloc(num_len + 1);
	num_view[num_len] = '\0';
	num_clone = num_len;

	for (; num; num_len--) {
		num_view[num_len - 1] = (num % 10) + '0';
		num /= 10;
	}

	if (minus_flag)
		num_view[0] = '-';

	return blist_from_str(num_view, num_clone);
}

struct BList *real_to_str(double num) {
	long before_dot = num;

	if (before_dot < 0)
		before_dot *= -1;

	struct BList *num_str = int_to_str(num);
	blist_add(num_str, '.');

	num -= before_dot;
	num *= 10;
	while (num) {
		before_dot = num;
		blist_add(num_str, before_dot + '0');

		num -= before_dot;
		num *= 10;
	}

	return num_str;
}

struct BList *type_to_blist_from_str(struct TypeExpr *type) {
	struct BList *str = new_blist(9), *tmp;
	const struct TypeWord *type_word;
	uint32_t i;
	long arr_len;

	if (type->code == TC_PTR) {
		blat(str, (uc *)TYPE_WORD_PTR.view, TYPE_WORD_PTR.view_len);
		blist_add(str, '_');
		tmp = type_to_blist_from_str(type->data.ptr_target);
		blat_blist(str, tmp);
		blist_clear_free(tmp);
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

		tmp = type_to_blist_from_str(arr_type(type));
		blat_blist(str, tmp);
		blist_clear_free(tmp);
		blist_add(str, '_');

		arr_len = (long)arr_size(type);
		if (arr_len == -1)
			blist_add(str, '~');
		else {
			tmp = int_to_str(arr_len);
			blat_blist(str, tmp);
			blist_clear_free(tmp);
		}

		blist_add(str, ']');
	} else if (type->code == TC_FUN) {
		// blat(str, (uc *)TYPE_WORD_FUN.view, TYPE_WORD_FUN.view_len);
		blist_add(str, '{');
		// -1 cuz lasr itteration after it
		for (i = 0; i < type->data.args_types->size - 1; i++) {
			tmp = type_to_blist_from_str(plist_get(type->data.args_types, i));
			blat_blist(str, tmp);
			blist_clear_free(tmp);
			blist_add(str, '_');
		}
		// this is last itteration, i just dont wanna do if in the loop above
		tmp = type_to_blist_from_str(plist_get(type->data.args_types, i));
		blat_blist(str, tmp);
		blist_clear_free(tmp);

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
	struct BList *signature = new_blist(32), *type_str;
	struct Arg *arg, *next_arg;
	uint32_t i;

	// add name
	blat_blist(signature, var->name->view);
	blist_add(signature, '_');
	// start fun type part
	blist_add(signature, '{');

	next_arg = plist_get(os, 1);
	if (!next_arg) // zero term
		goto skip_add_args;

	for (i = 1; next_arg;) {
		arg = next_arg;

		type_str = type_to_blist_from_str(arg->type); // gen type str
		blat_blist(signature, type_str);			  // add type str to str
		blist_clear_free(type_str);					  // free type str

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
	type_str = type_to_blist_from_str(
		plist_get(var->type->data.args_types,
				  var->type->data.args_types->size - 1)); // gen type str
	blat_blist(signature, type_str);					  // add type str to str
	blist_clear_free(type_str);							  // free type str

	// end fun type part
	blist_add(signature, '}');

	convert_blist_to_blist_from_str(signature);
	var->signature = signature;
}

void get_global_signature(struct PList *os, struct GlobVar *var) {
	struct BList *type_str;

	if (var->type->code == TC_FUN) {
		get_fun_signature_considering_args(os, var);
	} else {
		var->signature = new_blist(128);
		blat_blist(var->signature, var->name->view);
		blist_add(var->signature, '_');

		type_str = type_to_blist_from_str(var->type);
		blat_blist(var->signature, type_str);
		blist_clear_free(type_str);

		convert_blist_to_blist_from_str(var->signature);
	}
}

/*
this funstion only exists cuz its faster than comparison of
two function signature strings
*/
int are_types_equal(struct TypeExpr *t1, struct TypeExpr *t2) {
	if (t1->code != t2->code)
		return 0;

	// here codes are equal so code of t1 is aslo code of t2
	if (t1->code < TC_PTR)
		return 1;

	uint32_t res = 0;

	if (t1->code == TC_PTR) {
		res = are_types_equal(t1->data.ptr_target, t2->data.ptr_target);
	} else if (t1->code == TC_STRUCT || t1->code == TC_ENUM) {
		res = sc((char *)t1->data.name->st, (char *)t2->data.name->st);
	} else if (t1->code == TC_ARR) {
		res = are_types_equal(arr_type(t1), arr_type(t2)) &&
			  arr_size(t1) == arr_size(t2);
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
		free_type(type->data.ptr_target);

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
		if (sc((char *)cur->view->st, STR_STR_TW)) {
			texpr->code = TC_PTR;
			texpr->data.ptr_target = new_type_expr(TC_UINT8);

		} else if (1) {
			if (sc((char *)cur->view->st, STR_STRUCT_TW))
				texpr->code = TC_STRUCT;
			else if (sc((char *)cur->view->st, STR_ENUM_TW))
				texpr->code = TC_ENUM;
			else
				goto check_type_words;

			cur = absorb(p);
			expect(p, cur, ID);
			texpr->data.name = cur->view;

		} else {
		check_type_words:
			for (size_t i = 0; i < loa(TYPE_WORDS); i++) {
				tw = TYPE_WORDS + i;

				if (sc((char *)cur->view->st, tw->view)) {
					texpr->code = tw->code;
					goto ret_type_expr;
				}
			}
			eet(p->f, cur, NOT_A_TYPE_WORD, 0);
		}
	} else if (cur->code == MUL) {
		texpr->code = TC_PTR;
		consume(p); // consume *
		texpr->data.ptr_target = type_expr(p);
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
			if (cur->number < -1)
				eet(p->f, cur, WRONG_ARR_SIZE, 0);
			plist_add(texpr->data.arr, (void *)cur->number);
			consume(p); // consume arr size
			cur = pser_cur(p);
		} else
			eet(p->f, cur, ERR_WRONG_TOKEN_NUM_PAR_C_R, 0);

		expect(p, cur, PAR_C_R); // expect ]

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

			expect(p, pser_cur(p), PAR_R); // expect )

		} else if (cur->code == PAR_R) {
			if (texpr->data.args_types->size == 0)
				eet(p->f, cur, FUN_ZERO_ARGS, SUGGEST_ADD_ARGS);
		} else
			eet(p->f, cur, FUN_TYPE_END_OF_FILE, 0);
	} else
		eet(p->f, cur, NOT_A_TYPE_WORD, 0);

ret_type_expr:
	consume(p); // ] ) type_word struct_name str
	return texpr;
}
