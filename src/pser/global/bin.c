#include "../pser.h"

constr UNKNOWN_OPERATION = "Неизвестная операция.";
constr INVALID_OPERANDS_TYPES_FOR_THIS_OP =
	"Неверные типы поерандов для данной операции.";
constr CANT_MUL_STR_ON_VALUE_LESS_THAN_ZERO =
	"Строки можно умножать только на целые числа, что больше минус единицы.";
constr DIV_ON_ZERO = "Деление на ноль запрещено.";

// ###########################################################################################
#define int_fun(word, op) 															 		\
	struct GlobExpr *glob_##word##_two_ints(struct GlobExpr *l, struct GlobExpr *r) {		\
		l->tvar->num = l->tvar->num op r->tvar->num; 										\
		return l;																 			\
	}
// ###########################################################################################
#define num_fun(word, op) 																	\
	int_fun(word, op);																		\
	struct GlobExpr *glob_##word##_two_reals(struct GlobExpr *l, struct GlobExpr *r) {		\
		l->tvar->real = l->tvar->real op r->tvar->real;										\
		return l;																			\
	}																						\
	struct GlobExpr *glob_##word##_real_and_int(struct GlobExpr *l, struct GlobExpr *r) {	\
		l->tvar->real = l->tvar->real op r->tvar->num;										\
		return l;																			\
	}																						\
	struct GlobExpr *glob_##word##_int_and_real(struct GlobExpr *l, struct GlobExpr *r) {	\
		l->tvar->real = l->tvar->num op r->tvar->real;										\
		l->code = r->code;																	\
		return l;																			\
	}
// ###########################################################################################
// str
struct GlobExpr *glob_add_two_strs(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->view->size--; // remove last " of l
	r->tvar->view->st++;   // remove first " of r
	r->tvar->view->size--;
	blat_blist(l->tvar->view, r->tvar->view); // copy
	r->tvar->view->st--;					  // restore first " in r
	r->tvar->view->size++;
	convert_blist_to_blist_from_str(l->tvar->view);

	blat_blist(l->tvar->str, r->tvar->str);
	convert_blist_to_blist_from_str(l->tvar->str);
	return l;
}

struct GlobExpr *glob_add_int_and_str(struct GlobExpr *l, struct GlobExpr *r) {
	struct BList *num = int_to_str(l->tvar->num);
	struct BList *str = new_blist(num->size + r->tvar->str->size + 1);

	blat_blist(str, num);
	blat_blist(str, r->tvar->str);
	l->tvar->str = str;
	convert_blist_to_blist_from_str(l->tvar->str);

	str = new_blist(num->size + r->tvar->view->size + 1);
	blist_add(str, '\"'); // add "
	blat_blist(str, num); // add num
	r->tvar->view->st++;
	r->tvar->view->size--;
	blat_blist(str, r->tvar->view); // add view with "
	r->tvar->view->st--;
	r->tvar->view->size++;
	l->tvar->view = str;
	convert_blist_to_blist_from_str(l->tvar->view);

	l->code = r->code; // str code

	blist_clear_free(num);
	return l;
}

struct GlobExpr *glob_add_str_and_int(struct GlobExpr *l, struct GlobExpr *r) {
	struct BList *num = int_to_str(r->tvar->num);

	blat_blist(l->tvar->str, num);
	convert_blist_to_blist_from_str(l->tvar->str);

	l->tvar->view->size--; // remove last "
	blat_blist(l->tvar->view, num);
	blist_add(l->tvar->view, '"');
	convert_blist_to_blist_from_str(l->tvar->view);

	blist_clear_free(num);
	return l;
}

struct GlobExpr *glob_add_real_and_str(struct GlobExpr *l, struct GlobExpr *r) {
	struct BList *num = real_to_str(l->tvar->real);
	struct BList *str = new_blist(num->size + r->tvar->str->size + 1);

	blat_blist(str, num);
	blat_blist(str, r->tvar->str);
	l->tvar->str = str;
	convert_blist_to_blist_from_str(l->tvar->str);

	str = new_blist(num->size + r->tvar->view->size + 1);
	blist_add(str, '\"'); // add "
	blat_blist(str, num); // add num
	r->tvar->view->st++;
	r->tvar->view->size--;
	blat_blist(str, r->tvar->view); // add view with "
	r->tvar->view->st--;
	r->tvar->view->size++;
	l->tvar->view = str;
	convert_blist_to_blist_from_str(l->tvar->view);

	l->code = r->code; // str code

	blist_clear_free(num);
	return l;
}

struct GlobExpr *glob_add_str_and_real(struct GlobExpr *l, struct GlobExpr *r) {
	struct BList *num = real_to_str(r->tvar->real);

	blat_blist(l->tvar->str, num);
	convert_blist_to_blist_from_str(l->tvar->str);

	l->tvar->view->size--; // remove last "
	blat_blist(l->tvar->view, num);
	blist_add(l->tvar->view, '"');
	convert_blist_to_blist_from_str(l->tvar->view);

	blist_clear_free(num);
	return l;
}

struct GlobExpr *glob_mul_str_and_int(struct GlobExpr *l, struct GlobExpr *r) {
	long i = r->tvar->num;

	if (i == 0) {
		blist_clear_free(l->tvar->view);
		blist_clear_free(l->tvar->str);

		l->tvar->view = new_blist(3);
		blist_add(l->tvar->view, '"');
		blist_add(l->tvar->view, '"');
		convert_blist_to_blist_from_str(l->tvar->view);

		l->tvar->str = new_blist(2);
		convert_blist_to_blist_from_str(l->tvar->str);
		goto ret;
	}
	if (i == 1)
		goto ret;

	l->tvar->view->st++;
	l->tvar->view->size -= 2; // remove all ""
	struct BList *view_once = copy_blist(l->tvar->str);
	l->tvar->view->st--;
	l->tvar->view->size += 1; // restore first "
	struct BList *str_once = copy_blist(l->tvar->str);

	for (i--; i > 0; i--) {
		blat_blist(l->tvar->view, view_once);
		blat_blist(l->tvar->str, str_once);
	}
	blist_add(l->tvar->view, '"');
	convert_blist_to_blist_from_str(l->tvar->view);
	convert_blist_to_blist_from_str(l->tvar->str);

	blist_clear_free(view_once);
	blist_clear_free(str_once);
ret:
	return l;
}

// ###########################################################################################
num_fun(add, +)
num_fun(sub, -)
num_fun(mul, *)
num_fun(div, /)
num_fun(and, &&)
num_fun(or, ||)
num_fun(eque, ==)
num_fun(nequ, !=)
num_fun(less, <)
num_fun(more, >)
num_fun(lesse, <=)
num_fun(moree, >=)

int_fun(mod, %);
int_fun(bit_and, &);
int_fun(bit_or, |);
int_fun(bit_xor, ^);
int_fun(shl, <<);
int_fun(shr, >>);
// ###########################################################################################

#define is_ct_int(e) ((e)->code == CT_INT)
#define is_ct_real(e) ((e)->code == CT_REAL)
#define is_ct_str(e) (((e)->code == CT_STR || (e)->code == CT_STR_PTR))
#define is_ct_arr(e) (((e)->code == CT_ARR || (e)->code == CT_ARR_PTR))
#define is_ct_struct(e) (((e)->code == CT_STRUCT || (e)->code == CT_STRUCT_PTR))

#define is_int_zero(e) ((is_ct_int((e)) && (e)->tvar->num == 0))
#define is_real_zero(e) ((is_ct_real((e)) && (e)->tvar->real == 0.0))

// ###########################################################################################
#define only_ints(op_e, word)                                                  \
	if (op->code == (op_e)) {                                                  \
		if (is_ct_int(l) && is_ct_int(r))                                      \
			l = glob_##word##_two_ints(l, r);                                  \
		else                                                                   \
			eet(op, INVALID_OPERANDS_TYPES_FOR_THIS_OP, 0);					   \
	}                                                                          \
// ###########################################################################################
#define check_num_types(word)                                                  \
	if (is_ct_int(l) && is_ct_int(r))                                          \
		l = glob_##word##_two_ints(l, r);                                      \
	else if (is_ct_real(l) && is_ct_real(r))                                   \
		l = glob_##word##_two_reals(l, r);                                     \
	else if (is_ct_real(l) && is_ct_int(r))                                    \
		l = glob_##word##_real_and_int(l, r);                                  \
	else if (is_ct_int(l) && is_ct_real(r))                                    \
		l = glob_##word##_int_and_real(l, r);
// ###########################################################################################
#define only_nums(op_e, word) 												   \
	if (op->code == (op_e)) {												   \
		check_num_types(word)												   \
		else															 	   \
			eet(op, INVALID_OPERANDS_TYPES_FOR_THIS_OP, 0);			   \
	}

struct GlobExpr *global_bin(struct Pser *p, struct GlobExpr *l,
							struct GlobExpr *r, struct Token *op) {
	if (op->code == PLUS) {
		check_num_types(add)
		else if (is_ct_str(l) && is_ct_str(r)) l = glob_add_two_strs(l, r);
		else if (is_ct_int(l) && is_ct_str(r)) l = glob_add_int_and_str(l, r);
		else if (is_ct_str(l) && is_ct_int(r)) l = glob_add_str_and_int(l, r);
		else if (is_ct_real(l) && is_ct_str(r)) l = glob_add_real_and_str(l, r);
		else if (is_ct_str(l) && is_ct_real(r)) l = glob_add_str_and_real(l, r);
		else
			eet(op, INVALID_OPERANDS_TYPES_FOR_THIS_OP, 0);

	} else if (op->code == MUL) {
		check_num_types(mul)
		else if (is_ct_str(l) && is_ct_int(r)) {
			if (r->tvar->num < 0)
				eet(op, CANT_MUL_STR_ON_VALUE_LESS_THAN_ZERO, 0);

			l = glob_mul_str_and_int(l, r);
		} else
			eet(op, INVALID_OPERANDS_TYPES_FOR_THIS_OP, 0);

	} else if (op->code == DIV) {
		if (is_int_zero(r) || is_real_zero(r))
			eet(op, DIV_ON_ZERO, 0);

		check_num_types(div)
		else
			eet(op, INVALID_OPERANDS_TYPES_FOR_THIS_OP, 0);

	}
	else only_nums(EQUE, eque)
	else only_nums(NEQU, nequ)
	else only_nums(MINUS, sub)
	else only_nums(AND, and)
	else only_nums(OR, or)
	else only_nums(LESS, less)
	else only_nums(MORE, more)
	else only_nums(LESSE, lesse)
	else only_nums(MOREE, moree)
	else only_ints(MOD, mod)
	else only_ints(AMPER, bit_and)
	else only_ints(BIT_XOR, bit_xor)
	else only_ints(BIT_OR, bit_or)
	else only_ints(SHL, shl)
	else only_ints(SHR, shr)
	else
		eet(op, UNKNOWN_OPERATION, 0);

	free_glob_expr(r);
	return l;
}
