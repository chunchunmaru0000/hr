#include "../../gner.h"
#include <stdio.h>

constr ALLOWANCE_OF_INDEXATION =
	"Индексация разрешена только по массивам и указателям.";

#define add_or_sub                                                             \
	do {                                                                       \
		if (is_inc)                                                            \
			iprint_fun_text(SA_ADD);                                           \
		else                                                                   \
			iprint_fun_text(SA_SUB);                                           \
	} while (0)

long add_of_type(struct Token *tvar, struct TypeExpr *type) {
	if (is_int_type(type))
		return 1;
	if (type->code == TC_PTR)
		return unsafe_size_of_type(type->data.ptr_target);

	eet(tvar, "Нельзя '++' или '--' сюда.", 0);
	return 0;
}

void gen_dec_inc(Gg, struct LocalExpr *e, uc is_inc) {
	struct LocalExpr *trailed;

	if (is_mem(e)) {
		gen_mem_tuple(g, e);

		add_or_sub;
		mem_(g, e, 0);
		add_int_with_hex_comm(fun_text, add_of_type(e->tvar, e->type));
	} else if ((trailed = is_not_assignable_or_trailed(e))) {
		struct Reg *r1 = 0;
		struct BList *last_mem_str = 0;

		r1 = gen_to_reg_with_last_mem(g, e, trailed, &last_mem_str);
		add_or_sub;
		blat_ft(last_mem_str);
		add_int_with_hex_comm(fun_text, add_of_type(e->tvar, e->type));

		free_reg_rf_if_not_zero(r1);
		blist_clear_free(last_mem_str);
	} else
		eet(e->l->tvar, NOT_ASSIGNABLE, 0);
}

struct Reg *unary_dec_inc(Gg, struct LocalExpr *e, uc is_inc) {
	/*
	++a
	mov r, mem a
	add r unit
	mov mem a, r

	res is in r
	 */
	struct Reg *r;
	struct LocalExpr *trailed;

	if (is_mem(e)) {
		gen_mem_tuple(g, e);


		add_or_sub;

	} else if ((trailed = is_not_assignable_or_trailed(e))) {
		struct Reg *r1 = 0;
		struct BList *last_mem_str = 0;

		r1 = gen_to_reg_with_last_mem(g, e, trailed, &last_mem_str);
		add_or_sub;
		blat_ft(last_mem_str);


		free_reg_rf_if_not_zero(r1);
		blist_clear_free(last_mem_str);
	} else
		eet(e->l->tvar, NOT_ASSIGNABLE, 0);

	return r;
}

struct Reg *after_dec_inc(Gg, struct LocalExpr *e, uc is_inc) {
	/*
	a++
	mov r1, mem a
	mov r2, r1
	add r2 unit
	mov mem a, r2

	mov r1, mem a
	add r1 unit
	mov mem a, r1
	sub r1 unit

	res is in r1
	 */
}
