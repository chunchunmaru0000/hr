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

		free_register(r1);
		blist_clear_free(last_mem_str);
	} else
		eet(e->l->tvar, NOT_ASSIGNABLE, 0);
}

struct Reg *unary_dec_inc(Gg, struct LocalExpr *e, uc is_inc) {
	/*
	++a
	mov r, mem
	add r unit
	mov mem, r
	 */
	struct Reg *r1, *r2 = 0;
	struct LocalExpr *trailed;
	int e_size = unsafe_size_of_type(e->type);
	int unit = add_of_type(e->tvar, e->type);

	r1 = try_borrow_reg(e->tvar, g, e_size);

	if (is_mem(e)) {
		gen_mem_tuple(g, e);
		// mov r, mem
		op_reg_(MOV, r1);
		mem_enter(e, 0);
		// dec / inc r, unit
		add_or_sub;
		reg_(r1);
		add_int_with_hex_comm(fun_text, unit);
		// mov mem, r
		op_mem_(MOV, e, 0);
		reg_enter(r1);

	} else if ((trailed = is_not_assignable_or_trailed(e))) {
		struct BList *last_mem_str = 0;
		r2 = gen_to_reg_with_last_mem(g, e, trailed, &last_mem_str);

		// mov r, mem
		op_reg_(MOV, r1);
		last_mem_enter(last_mem_str);
		// dec / inc r, unit
		add_or_sub;
		reg_(r1);
		add_int_with_hex_comm(fun_text, unit);
		// mov mem, r
		op_last_mem_(MOV, last_mem_str);
		reg_enter(r1);

		blist_clear_free(last_mem_str);
	} else
		eet(e->l->tvar, NOT_ASSIGNABLE, 0);

	free_register(r2);
	return r1;
}

struct Reg *after_dec_inc(Gg, struct LocalExpr *e, uc is_inc) {
	/*
	a++
	mov r1, mem a
	mov r2, r1
	add r2, unit
	mov mem a, r2
	// mov r1, mem a
	// add r1 unit
	// mov mem a, r1
	// sub r1 unit
	 */
	struct Reg *r1, *r2 = 0, *r3 = 0;
	struct LocalExpr *trailed;
	int e_size = unsafe_size_of_type(e->type);
	int unit = add_of_type(e->tvar, e->type);

	r1 = try_borrow_reg(e->tvar, g, e_size);
	r2 = try_borrow_reg(e->tvar, g, e_size);

	if (is_mem(e)) {
		gen_mem_tuple(g, e);
		// mov r1, mem
		op_reg_(MOV, r1);
		mem_enter(e, 0);
		// mov r2, r1
		op_reg_reg(MOV, r2, r1);
		// dec / inc r2, unit
		add_or_sub;
		reg_(r2);
		add_int_with_hex_comm(fun_text, unit);
		// mov mem, r2
		op_mem_(MOV, e, 0);
		reg_enter(r2);

	} else if ((trailed = is_not_assignable_or_trailed(e))) {
		struct BList *last_mem_str = 0;
		r3 = gen_to_reg_with_last_mem(g, e, trailed, &last_mem_str);

		// mov r1, mem
		op_reg_(MOV, r1);
		last_mem_enter(last_mem_str);
		// mov r2, r1
		op_reg_reg(MOV, r2, r1);
		// dec / inc r2, unit
		add_or_sub;
		reg_(r2);
		add_int_with_hex_comm(fun_text, unit);
		// mov mem, r2
		op_last_mem_(MOV, last_mem_str);
		reg_enter(r2);

		blist_clear_free(last_mem_str);
	} else
		eet(e->l->tvar, NOT_ASSIGNABLE, 0);

	free_register(r2);
	free_register(r3);
	return r1;
}
