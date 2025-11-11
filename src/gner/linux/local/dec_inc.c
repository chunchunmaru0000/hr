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

#define mov_rax_li                                                             \
	isprint_ft(MOV_RAX);                                                       \
	var_(g, li, 0);                                                            \
	g->fun_text->size--, ft_add('\n');
#define mov_rax_gi                                                             \
	isprint_ft(MOV_RAX);                                                       \
	var_(g, 0, gi);                                                            \
	g->fun_text->size--, ft_add('\n');
#define ads(base, scale, disp)                                                 \
	add_or_sub;                                                                \
	sib_(g, item_size, (base), (scale), R_RAX, (long)(disp), 1);               \
	add_int_with_hex_comm(fun_text, unit);

void lv_li_indec(Gg, Lv lvar, Lv li, U, inced) {
	uc item_size = unsafe_size_of_type(arr_type(lvar->type));
	// mov     rax, isize [rbp index]
	// add     item_size [rbp item_size * rax lvar], int
	mov_rax_li;
	ads(R_RBP, item_size, lvar->name->view);
}
void lv_gi_indec(Gg, Lv lvar, Gv gi, U, inced) {
	uc item_size = unsafe_size_of_type(arr_type(lvar->type));
	// mov     rax, isize [index]
	// add     item_size [rbp item_size * rax lvar], int
	mov_rax_gi;
	ads(R_RBP, item_size, lvar->name->view);
}
void gv_li_indec(Gg, Gv gvar, Lv li, U, inced) {
	uc item_size = unsafe_size_of_type(arr_type(gvar->type));
	// mov     rax, isize [rbp index]
	// add     item_size [item_size * rax gvar], int
	mov_rax_li;
	ads(0, item_size, gvar->signature);
}
void gv_gi_indec(Gg, Gv gvar, Gv gi, U, inced) {
	uc item_size = unsafe_size_of_type(arr_type(gvar->type));
	// mov     rax, isize [rbp index]
	// add     item_size [item_size * rax gvar], int
	mov_rax_gi;
	ads(0, item_size, gvar->signature);
}
void lv_ii_indec(Gg, Lv lvar, Le i, U, inced) {
	uc item_size = unsafe_size_of_type(arr_type(lvar->type));
	// mov rax, index * item_size
	// add item_size [rax rbp lvar], int
	isprint_ft(MOV_RAX);
	add_int_with_hex_comm(fun_text, i->tvar->num * item_size);
	ads(R_RBP, 1, lvar->name->view);
	// ##################### TODO: in assembly compiler
	// add item_size [rbp lvar+index*item_size], int
}
void gv_ii_indec(Gg, Gv gvar, Le i, U, inced) {
	uc item_size = unsafe_size_of_type(arr_type(gvar->type));
	// mov rax, index * item_size
	// add item_size [rax gvar], int
	isprint_ft(MOV_RAX);
	add_int_with_hex_comm(fun_text, i->tvar->num * item_size);
	ads(0, 1, gvar->signature);
	// ##################### TO DO: in assembly compiler
	// add item_size [gvar+index*item_size], int
}

// var[var or int][++ / --]
void var_index_indec(Gg, struct LocalExpr *e, uc is_inc) {
	Lvar = e->l;
	Le index = e->r;
	declare_lvar_gvar;
	void *lgvar;
	get_assignee_size(g, var, &gvar, &lvar);

	struct TypeExpr *unit_type;
	long unit;

	if (var->type->code == TC_ARR) {
		unit_type = arr_type(var->type);
		unit = add_of_type(var->tvar, unit_type);

		if (lceep(index, VAR)) {
			if (lvar) {
				lgvar = lvar, lvar = 0, gvar = 0;
				get_assignee_size(g, index, &gvar, &lvar);
				lvar ? lv_li_indec(g, lgvar, lvar, unit, is_inc)
					 : lv_gi_indec(g, lgvar, gvar, unit, is_inc);
			} else {
				lgvar = gvar, lvar = 0, gvar = 0;
				get_assignee_size(g, index, &gvar, &lvar);
				lvar ? gv_li_indec(g, lgvar, lvar, unit, is_inc)
					 : gv_gi_indec(g, lgvar, gvar, unit, is_inc);
			}
		} else {
			if (lvar) {
				lgvar = lvar, lvar = 0, gvar = 0;
				lv_ii_indec(g, lgvar, index, unit, is_inc);
			} else {
				lgvar = gvar, lvar = 0, gvar = 0;
				gv_ii_indec(g, lgvar, index, unit, is_inc);
			}
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
