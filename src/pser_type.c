#include "pser.h"

const char *const ERR_WRONG_TOKEN_NUM_PAR_C_R =
	"Ожидалось целое число или скобка ']'.";
const char *const WRONG_ARR_SIZE =
	"Размера массива не может быть меньше -1, -1 значит любой размер.";
const char *const FUN_TYPE_END_OF_FILE =
	"Скобки типа функции не были закрыты и был достигнут конец файла.";
const char *const NOT_A_TYPE_WORD = "Ожидалось слово типа.";

const struct TypeWord TYPE_WORDS[] = {
	{"ч8", TC_INT8},	{"ц8", TC_UINT8},	{"ч16", TC_INT16},
	{"ц16", TC_UINT16}, {"ч32", TC_INT32},	{"ц32", TC_UINT32},
	{"в32", TC_FLOAT},	{"в64", TC_DOUBLE}, {"ч64", TC_INT64},
	{"ц64", TC_UINT64}, {"тлен", TC_VOID},
};
const char *const STR_STR_TW = "стр";
const char *const STR_STRUCT_TW = "лик";
const char *const STR_ENUM_TW = "счет";

struct TypeExpr *get_type_expr(enum TypeCode code) {
	struct TypeExpr *texpr = malloc(sizeof(struct TypeExpr));
	texpr->data.ptr_target = 0;
	texpr->code = code;

	return texpr;
}

struct TypeExpr *type_expr(struct Pser *p) {
	const struct TypeWord *tw;
	struct Token *cur = pser_cur(p);
	struct TypeExpr *texpr = get_type_expr(TC_VOID);

	if (cur->code == ID) {
		if (sc((char *)cur->view->st, STR_STR_TW)) {
			texpr->code = TC_PTR;
			texpr->data.ptr_target = get_type_expr(TC_UINT8);

		} else if (1) {
			if (sc((char *)cur->view->st, STR_STRUCT_TW))
				texpr->code = TC_STRUCT;
			else if (sc((char *)cur->view->st, STR_ENUM_TW))
				texpr->code = TC_INT32;
			else
				goto check_type_words;

			cur = absorb(p);
			expect(p, cur, ID);
			texpr->data.struct_name = cur->view;
			// TODO: maybe do enum name later for type safety

		} else {
		check_type_words:
			for (size_t i = 0; i < loa(TYPE_WORDS); i++) {
				tw = TYPE_WORDS + i;

				if (sc((char *)cur->view->st, tw->view)) {
					texpr->code = tw->code;
					goto ret_type_expr;
				}
			}
			ee(p->f, cur->p, NOT_A_TYPE_WORD);
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
				ee(p->f, cur->p, WRONG_ARR_SIZE);
			plist_add(texpr->data.arr, (void *)cur->number);
			consume(p); // consume arr size
			cur = pser_cur(p);
		} else
			ee(p->f, cur->p, ERR_WRONG_TOKEN_NUM_PAR_C_R);

		expect(p, cur, PAR_C_R); // expect ]

	} else if (cur->code == PAR_L) {
		texpr->code = TC_FUN;
		texpr->data.fun = new_plist(2);

		cur = absorb(p);
		while (cur->code != PAR_R && cur->code != EXCL && cur->code != EF) {
			plist_add(texpr->data.arr, type_expr(p));
			cur = pser_cur(p);
		}
		if (cur->code == EXCL) {
			consume(p); // consume !
			plist_add(texpr->data.arr, type_expr(p));
			expect(p, cur, PAR_R); // expect )
		} else if (cur->code == PAR_R) {
			// nothing
		} else
			ee(p->f, cur->p, FUN_TYPE_END_OF_FILE);
	}

ret_type_expr:
	consume(p); // ] ) type_word struct_name str
	return texpr;
}
