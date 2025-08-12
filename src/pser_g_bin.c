#include "pser.h"

// ###########################################################################################
// 											+
// ###########################################################################################
// int and real
struct GlobExpr *glob_add_two_ints(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->num += r->tvar->num;
	return l;
}
struct GlobExpr *glob_add_two_reals(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real += r->tvar->real;
	return l;
}
struct GlobExpr *glob_add_real_and_int(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real += r->tvar->num;
	return l;
}
struct GlobExpr *glob_add_int_and_real(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real = l->tvar->num + l->tvar->real;
	l->code = r->code;
	return l;
}
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
// ###########################################################################################
// 											-
// ###########################################################################################
struct GlobExpr *glob_sub_two_ints(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->num -= r->tvar->num;
	return l;
}
struct GlobExpr *glob_sub_two_reals(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real -= r->tvar->real;
	return l;
}
struct GlobExpr *glob_sub_real_and_int(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real -= r->tvar->num;
	return l;
}
struct GlobExpr *glob_sub_int_and_real(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->num -= r->tvar->real;
	return l;
}
// ###########################################################################################
// 											*
// ###########################################################################################
struct GlobExpr *glob_mul_two_ints(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->num *= r->tvar->num;
	return l;
}
struct GlobExpr *glob_mul_two_reals(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real *= r->tvar->real;
	return l;
}
struct GlobExpr *glob_mul_real_and_int(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real *= r->tvar->num;
	return l;
}
struct GlobExpr *glob_mul_int_and_real(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real = l->tvar->num * l->tvar->real;
	l->code = r->code;
	return l;
}
// ###########################################################################################
// 											/
// ###########################################################################################
#define is_int_zero(e) ( (e)->tvar->num == 0 )
#define is_real_zero(e) ( (e)->tvar->real == 0.0 )

struct GlobExpr *glob_div_two_ints(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->num /= r->tvar->num;
	return l;
}
struct GlobExpr *glob_div_two_reals(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real /= r->tvar->real;
	return l;
}
struct GlobExpr *glob_div_real_and_int(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real /= r->tvar->num;
	return l;
}
struct GlobExpr *glob_div_int_and_real(struct GlobExpr *l, struct GlobExpr *r) {
	l->tvar->real = l->tvar->num / l->tvar->real;
	l->code = r->code;
	return l;
}
// ###########################################################################################
//
// ###########################################################################################

#define is_ct_int(e) ((e)->code == CT_INT)
#define is_ct_real(e) ((e)->code == CT_REAL)
#define is_ct_str(e) (((e)->code == CT_STR || (e)->code == CT_STR_PTR))
#define is_ct_arr(e) (((e)->code == CT_ARR || (e)->code == CT_ARR_PTR))
#define is_ct_struct(e) (((e)->code == CT_STRUCT || (e)->code == CT_STRUCT_PTR))

struct GlobExpr *global_bin(struct Pser *p, struct GlobExpr *l,
							  struct GlobExpr *r, struct Token *op) {
	if (op->code == PLUS) {
		// int and real
		if (is_ct_int(l) && is_ct_int(r))
			l = glob_add_two_ints(l, r);
		else if (is_ct_real(l) && is_ct_real(r))
			l = glob_add_two_reals(l, r);
		else if (is_ct_real(l) && is_ct_int(r))
			l = glob_add_real_and_int(l, r);
		else if (is_ct_int(l) && is_ct_real(r))
			l = glob_add_int_and_real(l, r);
		// str
		else if (is_ct_str(l) && is_ct_str(r))
			l = glob_add_two_strs(l, r);
		else if (is_ct_int(l) && is_ct_str(r))
			l = glob_add_int_and_str(l, r);
		else if (is_ct_str(l) && is_ct_int(r))
			l = glob_add_str_and_int(l, r);
		else if (is_ct_real(l) && is_ct_str(r))
			l = glob_add_real_and_str(l, r);
		else if (is_ct_str(l) && is_ct_real(r))
			l = glob_add_str_and_real(l, r);
		//
		else
			exit(220); // just for now later do it not as a черт
	} else if (op->code == MINUS) {
		if (is_ct_int(l) && is_ct_int(r))
			l = glob_sub_two_ints(l, r);
		else if (is_ct_real(l) && is_ct_real(r))
			l = glob_sub_two_reals(l, r);
		else if (is_ct_real(l) && is_ct_int(r))
			l = glob_sub_real_and_int(l, r);
		else if (is_ct_int(l) && is_ct_real(r))
			l = glob_sub_int_and_real(l, r);
		else
			exit(219);
	} else if (op->code == MUL) {
		if (is_ct_int(l) && is_ct_int(r))
			l = glob_mul_two_ints(l, r);
		else if (is_ct_real(l) && is_ct_real(r))
			l = glob_mul_two_reals(l, r);
		else if (is_ct_real(l) && is_ct_int(r))
			l = glob_mul_real_and_int(l, r);
		else if (is_ct_int(l) && is_ct_real(r))
			l = glob_mul_int_and_real(l, r);
		else
			exit(218);
	} else if (op->code == DIV) {
		if (is_ct_int(r) && is_int_zero(r))
			exit(216);
		if (is_ct_real(r) && is_rael_zero(r))
			exit(215);
		
		if (is_ct_int(l) && is_ct_int(r))
			l = glob_div_two_ints(l, r);
		else if (is_ct_real(l) && is_ct_real(r))
			l = glob_div_two_reals(l, r);
		else if (is_ct_real(l) && is_ct_int(r))
			l = glob_div_real_and_int(l, r);
		else if (is_ct_int(l) && is_ct_real(r))
			l = glob_div_int_and_real(l, r);
		else
			exit(217);
	}

	free_glob_expr(r);
	return l;
}
