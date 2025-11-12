#include "../../gner.h"

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
	type_for_add = type_for_add->data.ptr_target;

	//	   mov	   rdx, var_ptr	// rdx is var value which is ptr
	isprint_ft(MOV_RAX);
	var_(g, lvar, gvar);
	g->fun_text->size--; // remove space after var_
	fun_text_add('\n');
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

		if (lvar) {
			item_size = unsafe_size_of_type(arr_type(lvar->type));
			base = R_RBP;
		} else {
			item_size = unsafe_size_of_type(arr_type(gvar->type));
			base = 0;
		}

		if (lceep(index, VAR)) {
			disp_str = lvar ? lvar->name->view : gvar->signature;
			lvar = 0, gvar = 0, get_assignee_size(g, index, &gvar, &lvar);

			mov_reg_var(g, R_RAX, lvar, gvar);

			add_or_sub;
			sib_(g, item_size, base, item_size, R_RAX, (long)(disp_str), 1);
			add_int_with_hex_comm(fun_text, unit);
		} else {
			if (gvar) {
				// ##################### TODO: in assembly compiler
				// add item_size [gvar+index*item_size], int
				mov_reg_(g, R_RAX);
				add_int_with_hex_comm(fun_text, index->tvar->num * item_size);

				add_or_sub;
				sib_(g, item_size, base, 1, R_RAX, (long)gvar->signature, 1);
			} else {
				add_or_sub;
				sib_(g, item_size, base, 1, 0,
					 lvar->stack_pointer + index->tvar->num * item_size, 0);
			}
			add_int_with_hex_comm(fun_text, unit);
		}
	} else {
	}

	// ; vs is var size
	if (var->type->code == TC_ARR) {
		if (lvar) {
			if (lceep(index, INT)) {
				// lea rax, [rbp index]
				// add vs[rax var], int

				// add vs[rbp var+index], int

			} else {
				// mov     rax, isize [rbp index]
				// add     arr_item_size [arr_item_size rax rbp var], int
			}
		} else {
			if (lceep(index, INT)) {
				// mov rax, var
				// add vs[rax index], int

				// add vs[var+index], int

			} else {
				// mov     rax, isize [rbp index]
				// add     arr_item_size [arr_item_size rax rbp var], int
			}
		}
	} else if (var->type->code == TC_PTR) {
		if (lceep(index, INT)) {
			// mov     rax, vsize [rbp var]
			// add 	   vsize [rax + vsize * index], int

		} else {
			// mov     rax, isize [rbp index]
			// lea 	   rax, [vsize rax] 		// rax is
			// add     rax, qword [rbp var] 	// qword cuz index from ptr
			// add     vsize [rax], int
		}
	} else
		eet(e->tvar, ALLOWANCE_OF_INDEXATION, 0);
}

void any_indec(Gg, uc is_inc) {
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
	} else
		any_indec(g, is_inc);
}
