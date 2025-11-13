#include "../../gner.h"
#include <stdio.h>

#define Lvar struct LocalExpr *var
#define Lv struct LocalVar *
#define Gv struct GlobVar *
#define Le struct LocalExpr *
#define LeE struct LocalExpr *e
#define U long unit
#define inced uc is_inc

constr ALLOWANCE_OF_INDEXATION =
	"Индексация разрешена только по массивам и указателям.";

#define let_lvar_gvar_inc let_lvar_gvar, uc is_inc
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

// var[++ / --]
void var_indec(Gg, Lvar, uc is_inc) {
	declare_lvar_gvar;
	get_assignee_size(g, var, &gvar, &lvar);

	add_or_sub;
	var_(g, lvar, gvar);
	add_int_with_hex_comm(fun_text, add_of_type(var->tvar, lvar_gvar_type()));
}

// (*var)[++ / --]
void var_ptr_indec(Gg, Lvar, uc is_inc) {
	declare_lvar_gvar;
	get_assignee_size(g, var, &gvar, &lvar);

	struct TypeExpr *type_for_add = lvar_gvar_type();
	if (type_for_add->code != TC_PTR)
		eet(var->tvar, EXPECTED_PTR_TYPE, 0);
	type_for_add = ptr_targ(type_for_add);

	//	   mov	   rdx, var_ptr	// rdx is var value which is ptr
	mov_reg_var(g, R_RAX, lvar, gvar);
	//     add/sub size_of_add_of_type [rax], int
	long aot = add_of_type(var->tvar, type_for_add);
	add_or_sub;
	if (aot == 1) // means ptr to value
		blat_fun_text(size_str(unsafe_size_of_type(type_for_add)));
	else // means ptr to ptr
		blat_fun_text(size_str(QWORD));

	sprint_ft(OFF_RAX);
	add_int_with_hex_comm(fun_text, aot);
}

// var[var or int][++ / --]
void var_index_indec(Gg, struct LocalExpr *e, uc is_inc) {
	Lvar = e->l;
	Le index = e->r;
	struct Reg *second_reg;
	declare_lvar_gvar;
	get_assignee_size(g, var, &gvar, &lvar);

	struct TypeExpr *unit_type;
	long unit;
	uc item_size;
	struct BList *disp_str;
	enum RegCode base;

	if (var->type->code == TC_ARR) {
		unit_type = arr_type(var->type);
		unit = add_of_type(var->tvar, unit_type);

		item_size = unsafe_size_of_type(unit_type);
		base = lvar ? R_RBP : 0;

		if (lceep(index, VAR)) {
			disp_str = lvar ? lvar->name->view : gvar->signature;

			lvar = 0, gvar = 0, get_assignee_size(g, index, &gvar, &lvar);
			mov_reg_var(g, R_RAX, lvar, gvar);

			add_or_sub;
			sib_(item_size, base, item_size, R_RAX, (long)(disp_str), 1);
			add_int_with_hex_comm(fun_text, unit);
		} else {
			disp_str = int_to_str(index->tvar->num * item_size);
			blist_add(disp_str, '+');
			blat_blist(disp_str, gvar ? gvar->signature : lvar->name->view);
			add_or_sub;
			sib_(item_size, base, 0, 0, (long)disp_str, 1);
			add_int_with_hex_comm(fun_text, unit);
		}
	} else if (var->type->code == TC_PTR) {
		unit_type = ptr_targ(var->type);
		unit = add_of_type(var->tvar, unit_type);

		item_size = unsafe_size_of_type(unit_type);
		base = lvar ? R_RBP : 0;

		if (lceep(index, VAR)) {
			if ((second_reg = borrow_basic_reg(g->cpu, QWORD))) {
				mov_reg_var(g, second_reg->reg_code, lvar, gvar);

				lvar = 0, gvar = 0, get_assignee_size(g, index, &gvar, &lvar);
				mov_reg_var(g, R_RAX, lvar, gvar);

				add_or_sub;
				sib_(item_size, second_reg->reg_code, item_size, R_RAX, 0, 0);
				add_int_with_hex_comm(fun_text, unit);

				free_reg_family(second_reg->rf);
				return;
			}

			disp_str = lvar ? lvar->name->view : gvar->signature;

			lvar = 0, gvar = 0, get_assignee_size(g, index, &gvar, &lvar);
			mov_reg_var(g, R_RAX, lvar, gvar);

			// lea 	   rax, qword [vsize rax] 	// rax is index
			op_reg_(LEA, R_RAX);
			sib(g, QWORD, 0, item_size, R_RAX, 0, 0), ft_add('\n');
			// add     rax, qword [rbp var] 	// qword cuz index from ptr
			op_reg_(ADD, R_RAX);
			sib(g, QWORD, base, 0, 0, (long)disp_str, 1), ft_add('\n');
			// add     item_size [rax], int
			add_or_sub;
			sib_(item_size, R_RAX, 0, 0, 0, 0);
			add_int_with_hex_comm(fun_text, unit);

		} else {
			mov_reg_var(g, R_RAX, lvar, gvar);
			add_or_sub;
			sib_(item_size, R_RAX, 0, 0, index->tvar->num * item_size, 0);
			add_int_with_hex_comm(fun_text, unit);
		}
	} else
		eet(e->tvar, ALLOWANCE_OF_INDEXATION, 0);
}

// var-[> / @]field[++ / --]
void var_field_indec(Gg, struct LocalExpr *e, uc is_inc) {
	// from define_struct_field_type_type
	struct Arg *feld = (struct Arg *)e->tvar->num;
	struct LocalExpr *var = e->l;

	struct TypeExpr *unit_type = feld->type;
	long unit = add_of_type(var->tvar, unit_type);
	int unit_size = feld->arg_size;

	declare_lvar_gvar;
	get_assignee_size(g, var, &gvar, &lvar);

	if (lcea(FIELD)) {
		if (lvar) {
			// add unit_size [rbp lvar+offset], unit
			add_or_sub;
			sib_(unit_size, R_RBP, 0, 0, lvar->stack_pointer + feld->offset, 0);
		} else {
			// add unit_size [gvar+offset], unit
			struct BList *disp = int_to_str(feld->offset);
			blist_add(disp, '+');
			blat_blist(disp, gvar->signature);
			add_or_sub;
			sib_(unit_size, 0, 0, 0, (long)disp, 1);
		}
		add_int_with_hex_comm(fun_text, unit);
	} else {
		// mov     rax, qword [var]
		// add     unit_size [rax + offset], unit
		mov_reg_var(g, R_RAX, lvar, gvar);
		add_or_sub;
		sib_(unit_size, R_RAX, 0, 0, feld->offset, 0);
		add_int_with_hex_comm(fun_text, unit);
	}
}

void any_indec(Gg, struct LocalExpr *e, uc is_inc) {
	// addr = gen_left_side_addr(g, inced);
	add_or_sub;
	// blat_fun_text(inced_addr->name);
	fun_text_add(' ');
	add_int_with_hex_comm(fun_text, 1);
	fun_text_add('\n');
}

void gen_dec_inc(Gg, struct LocalExpr *e, uc is_inc) {
	if (lcep(VAR)) {
		// var[++ / --]
		var_indec(g, e, is_inc);

	} else if (lceu(ADDR) && lceep(e->l, VAR)) {
		// *var[++ / --]
		var_ptr_indec(g, e->l, is_inc);

	} else if (lcea(INDEX) && lceep(e->l, VAR) &&
			   (lceep(e->r, VAR) || lceep(e->r, INT))) {
		// var[var or int][++ / --]
		var_index_indec(g, e, is_inc);

	} else if ((lcea(FIELD_OF_PTR) || lcea(FIELD)) && lceep(e->l, VAR)) {
		// var-[> / @]field[++ / --]
		var_field_indec(g, e, is_inc);

	} else
		any_indec(g, e, is_inc);
}
