#include "../../gner.h"
#include <stdio.h>

sa(LET_8, "пусть байт ");
sa(LET_16, "пусть дбайт ");
sa(LET_32, "пусть чбайт ");
sa(LET_64, "пусть вбайт ");
sa(REZERV_ZERO, "запас 0 ");

void clear_current_inst_value_labels_to(struct Gner *g, struct BList *label) {
	struct GlobVar *this_e_var;
	struct BList *freed = 0;
	uint32_t i;

	// clear current vars value_label if it was set during compilation
	for (i = 0; i < g->current_inst->os->size; i++) {
		this_e_var = plist_get(g->current_inst->os, i);

		if (this_e_var->value_label) {

			if (this_e_var->value_label && freed != this_e_var->value_label) {
				freed = this_e_var->value_label;
				blist_clear_free(this_e_var->value_label);
			}
			this_e_var->value_label = label;
		}
	}
}

struct BList *lay_down_int(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(32);
	enum TypeCode code = e->type->code;

	if (code == TC_INT8 || code == TC_UINT8)
		iprint_gen(SA_LET_8);
	else if (code == TC_INT16 || code == TC_UINT16)
		iprint_gen(SA_LET_16);
	else if (code == TC_INT32 || code == TC_UINT32 || code == TC_ENUM)
		iprint_gen(SA_LET_32);
	else if (code == TC_INT64 || code == TC_UINT64 || code == TC_VOID)
		iprint_gen(SA_LET_64);
	else {
		printf("#ERR_INFO. e->type->code was %d\n", code);
		exit(223);
	}

	int_add(generated, e->tvar->num);
	print_gen(SA_START_COMMENT); // \t;
	hex_int_add(generated, e->tvar->num);
	gen_add('\n');

	return generated;
}
struct BList *lay_down_real(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(32);

	if (e->type->code == TC_DOUBLE)
		iprint_gen(SA_LET_64);
	else
		iprint_gen(SA_LET_32);

	real_add(generated, e->tvar->real);
	gen_add('\n');

	return generated;
}
struct BList *lay_down_str(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(32);

	iprint_gen(SA_LET_8);
	blat_blist(generated, e->tvar->view);
	print_gen(SA_ZERO_TERMINATOR);

	return generated;
}
struct BList *lay_down_gptr(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(32);

	iprint_gen(SA_LET_64);
	blat_blist(generated, e->from->signature);
	gen_add('\n');

	return generated;
}

uc need_to_gen_ptr = 1;

struct BList *lay_down_obj(struct Gner *g, struct GlobExpr *e) {

	struct BList *generated = new_blist(64), *tmp_gen;
	struct PList *labels = new_plist(2);
	struct GlobExpr *glob;
	uint32_t i;

	for (i = 0; i < e->globs->size; i++) {
		glob = plist_get(e->globs, i);

		if (glob->code == CT_ARR_PTR || glob->code == CT_STRUCT_PTR) {
			need_to_gen_ptr = 0;

			g->indent_level += 2;

			// save generared code to labels
			tmp_gen = gen_glob_expr_linux(g, glob);
			plist_add(labels, tmp_gen);

			g->indent_level -= 2;

			// save label
			tmp_gen = take_label(g, LC_PTR);
			plist_add(labels, tmp_gen);

			// lay label
			iprint_gen(SA_LET_64);
			blat_blist(generated, tmp_gen);
			gen_add('\n');

			need_to_gen_ptr = 1;
		} else {
			// just gen
			tmp_gen = gen_glob_expr_linux(g, glob);
			copy_to_fst_and_clear_snd(generated, tmp_gen);
		}
	}

	g->indent_level += 1;

	for (i = 0; i < labels->size; i += 2) {
		indent_line(g, generated);
		// decalre label
		tmp_gen = plist_get(labels, i + 1); // label
		copy_to_fst_and_clear_snd(generated, tmp_gen);
		print_gen(SA_LABEL_END); // :

		// lay generared code
		tmp_gen = plist_get(labels, i); // code
		copy_to_fst_and_clear_snd(generated, tmp_gen);
	}

	g->indent_level -= 1;

	plist_free(labels);
	clear_current_inst_value_labels_to(g, 0);
	return generated;
}
struct BList *lay_down_obj_ptr(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(64), *ptr, *tmp_gen;
	uc was_need_to_gen_ptr = need_to_gen_ptr;

	if (was_need_to_gen_ptr) {
		ptr = take_label(g, LC_PTR);
		// declare label
		iprint_gen(SA_LET_64);
		blat_blist(generated, ptr);
		gen_add('\n');

		// lay label
		blat_blist(generated, ptr);
		print_gen(SA_LABEL_END); // :
	}

	tmp_gen = lay_down_obj(g, e);
	copy_to_fst_and_clear_snd(generated, tmp_gen);

	if (was_need_to_gen_ptr) {
		clear_current_inst_value_labels_to(g, ptr);
		// blist_clear_free(ptr); no need
	}

	need_to_gen_ptr = was_need_to_gen_ptr;
	return generated;
}
struct BList *lay_down_str_ptr(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(64);
	iprint_gen(SA_LET_64);

	if (!e->from) {
	add_value_ptr_to_this_e_var:;

		struct BList *ptr = take_label(g, LC_PTR);
		struct GlobVar *this_e_var;

		for (uint32_t j = 0; j < g->current_inst->os->size; j++) {
			this_e_var = plist_get(g->current_inst->os, j);
			this_e_var->value_label = ptr;
		}

		blat_blist(generated, ptr);
		gen_add('\n');

		blat_aprol(ptr);
		print_aprol(SA_LABEL_END); // :

		aprol_add('\t');
		print_aprol(SA_LET_8);
		blat_aprol(e->tvar->view);
		print_aprol(SA_ZERO_TERMINATOR);

	} else if (e->from) {
		if (e->from->value_label) {
			blat_blist(generated, e->from->value_label);
			gen_add('\n');
		} else {
			// not sure about this but
			// even if its possible it seems to be the case
			goto add_value_ptr_to_this_e_var;
		}
	}

	return generated;
}

struct BList *lay_down_zero(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(64);

	// e->tvar->num of CT_ZERO is arg_size
	struct BList *times = int_to_str(e->tvar->num);

	iprint_gen(SA_LET_8);
	print_gen(SA_REZERV_ZERO);
	copy_to_fst_and_clear_snd(generated, times);
	gen_add('\n');

	return generated;
}

struct BList *gen_glob_expr_linux(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated;
	enum CT_Code code = e->code;

	if (code != CT_ARR && e->type->code == TC_ARR) {
		e->type->code = arr_type(e->type)->code;
		generated = gen_glob_expr_linux(g, e);
		e->type->code = TC_ARR;
	} else if (code == CT_INT)
		generated = lay_down_int(g, e);
	else if (code == CT_REAL)
		generated = lay_down_real(g, e);
	else if (code == CT_STR)
		generated = lay_down_str(g, e);
	else if (code == CT_GLOBAL_PTR)
		generated = lay_down_gptr(g, e);
	else if (code == CT_STR_PTR)
		generated = lay_down_str_ptr(g, e);
	else if (code == CT_ARR_PTR || code == CT_STRUCT_PTR)
		generated = lay_down_obj_ptr(g, e);
	else if (code == CT_ARR || code == CT_STRUCT)
		generated = lay_down_obj(g, e);
	else if (code == CT_ZERO)
		generated = lay_down_zero(g, e);

	return generated;
}
