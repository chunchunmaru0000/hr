#include "../../gner.h"
#include <stdio.h>

struct BList *size_str(uc size) {
	if (size == BYTE)
		return copy_blist_from_str((char *)SA_BYTE);
	if (size == WORD)
		return copy_blist_from_str((char *)SA_WORD);
	if (size == DWORD)
		return copy_blist_from_str((char *)SA_DWORD);
	if (size == QWORD)
		return copy_blist_from_str((char *)SA_QWORD);
	printf("size %d\n", size);
	exit(127);
}

void gen_int(struct Gner *g, struct LocalExpr *e) {
	long num = e->tvar->num;

	if (num == 0) {
		iprint_fun_text(SA_STR_XOR_EAX_EAX);
	} else {
	}
	fun_text_add('\n');
}

void gen_real(struct Gner *g, struct LocalExpr *e);
void gen_var(struct Gner *g, struct LocalExpr *e);
void gen_assign(struct Gner *g, struct LocalExpr *e);

#define colored(name, code) ("\x1B[" #code "m")
constr colours[8] = {
	colored(RED, 31),	 colored(GREEN, 32),   colored(YELLOW, 33),
	colored(WH_RED, 91), colored(MAGENTA, 95), colored(CYAN, 36),
	colored(WHITE, 37),	 colored(WH_BLUE, 96),
};
int color_level = 0;
#define take_color_level() (colours[color_level++ % loa(colours)])
#define remove_color_level() (colours[--color_level % loa(colours)])
void print_le(struct LocalExpr *e, int with_n) {
	u32 i;

	if (e->tuple) {
		printf("%s(%s", take_color_level(), COLOR_RESET);
		for (i = 0; i < e->tuple->size; i++) {
			print_le(plist_get(e->tuple, i), 0);
			printf(", ");
		}
	}

	if (is_bin_le(e) || lceb(ASSIGN)) {
		printf("%s(%s", take_color_level(), COLOR_RESET);
		print_le(e->l, 0);
		printf(" %s ", vs(e->tvar));
		print_le(e->r, 0);
		printf("%s)%s", remove_color_level(), COLOR_RESET);
	} else if (lceb(TERRY)) {
		printf("%s{%s", take_color_level(), COLOR_RESET);
		print_le(e->l, 0);
		printf(" ? ");
		print_le(e->r, 0);
		printf(" : ");
		print_le(e->co.cond, 0);
		printf("%s}%s", remove_color_level(), COLOR_RESET);
	} else if (lcea(INDEX)) {
		print_le(e->l, 0);
		printf("%s[%s", take_color_level(), COLOR_RESET);
		print_le(e->r, 0);
		printf("%s]%s", remove_color_level(), COLOR_RESET);
	} else if (lcea(FIELD_OF_PTR) || lcea(FIELD)) {
		print_le(e->l, 0);
		printf("%s%s", vs(e->tvar), vs((struct Token *)e->r));
	} else if (lcea(CALL)) {
		print_le(e->l, 0);
		printf("%s(%s", take_color_level(), COLOR_RESET);
		if (e->co.ops->size) {
			for (i = 0; i < e->co.ops->size - 1; i++) {
				print_le(plist_get(e->co.ops, i), 0);
				printf(", ");
			}
			print_le(plist_get(e->co.ops, i), 0);
		}
		printf("%s)%s", remove_color_level(), COLOR_RESET);
	} else if (lce(BOOL)) {
		printf("бул%s(%s", take_color_level(), COLOR_RESET);
		print_le(e->l, 0);
		printf("%s)%s", remove_color_level(), COLOR_RESET);
	} else if (is_unary(e)) {
		printf("%s(%s%s", take_color_level(), COLOR_RESET, vs(e->tvar));
		print_le(e->l, 0);
		printf("%s)%s", remove_color_level(), COLOR_RESET);
	} else if (lcea(INC) || lcea(DEC)) {
		print_le(e->l, 0);
		printf("%s", vs(e->tvar));
	} else {
		printf("%s", vs(e->tvar));
	}

	if (e->tuple)
		printf("%s)%s", remove_color_level(), COLOR_RESET);
	if (with_n)
		putchar('\n');
}

#define just_char(c) (blist_add(out, (c)))
#define print_tvar(e) (blat_blist(out, (e)->tvar->view))
#define print_str(str) (badd_str(out, (str)))
#define print_orher(o)                                                         \
	do {                                                                       \
		other = bprint_le((o), 0);                                             \
		blat_blist(out, other);                                                \
		blist_clear_free(other);                                               \
	} while (0)

struct BList *bprint_le(struct LocalExpr *e, int with_n) {
	struct BList *out = new_blist(64), *other;
	u32 i;

	if (e->tuple) {
		just_char('(');
		for (i = 0; i < e->tuple->size; i++) {
			print_orher(plist_get(e->tuple, i));
			print_str(", ");
		}
	}

	if (is_bin_le(e) || lceb(ASSIGN)) {
		just_char('(');
		print_orher(e->l);
		just_char(' ');
		print_tvar(e);
		just_char(' ');
		print_orher(e->r);
		just_char(')');
	} else if (lceb(TERRY)) {
		just_char('{');
		print_orher(e->l);
		print_str(" ? ");
		print_orher(e->r);
		print_str(" : ");
		print_orher(e->co.cond);
		just_char('}');
	} else if (lcea(INDEX)) {
		print_orher(e->l);
		just_char('[');
		print_orher(e->r);
		just_char(']');
	} else if (lcea(FIELD_OF_PTR) || lcea(FIELD)) {
		print_orher(e->l);
		print_tvar(e);
		print_str(vs((struct Token *)e->r));
	} else if (lcea(CALL)) {
		print_orher(e->l);
		just_char('(');
		if (e->co.ops->size) {
			for (i = 0; i < e->co.ops->size - 1; i++) {
				print_orher(plist_get(e->co.ops, i));
				print_str(", ");
			}
			print_orher(plist_get(e->co.ops, i));
		}
		just_char(')');
	} else if (lce(BOOL)) {
		print_str("бул(");
		print_orher(e->l);
		just_char(')');
	} else if (is_unary(e)) {
		just_char('(');
		print_tvar(e);
		print_orher(e->l);
		just_char(')');
	} else if (lcea(INC) || lcea(DEC)) {
		print_orher(e->l);
		print_tvar(e);
	} else {
		print_tvar(e);
	}

	if (e->tuple)
		just_char(')');
	if (with_n)
		blist_add(out, '\n');

	return out;
}

void gen_local_expr_linux(Gg, struct LocalExpr *e) {
	struct BList *expr_view;
	// if e wa cut from other then e can have a tuple
	gen_tuple_of(g, e);

	indent_line(g, g->fun_text), ft_add(';'), ft_add(' ');
	expr_view = zero_term_blist(bprint_le(e, 1));
	blat_fun_text(expr_view);
	blist_clear_free(expr_view);

	print_le(e, 1);

	if (lceb(ASSIGN))
		gen_assign(g, e);
	else if (lceu(INC) || lcea(INC) || lceu(DEC) || lcea(DEC))
		gen_dec_inc(g, e->l, lceu(INC) || lcea(INC));
	// else if (lcea(CALL))
	// 	gen_call(g, e);
	// else if (lceb(TERRY))
	//	gen_terry(g, e);
	else
		// 	exit(159);
		printf("### NOT GEN LOCAL EXPR INFO: e->code == %d\n", e->code);
}

void gen_local_expr_inst_linux(struct Gner *g, struct Inst *in) {
	struct LocalExpr *e;
	struct PList *es;
	u32 i;

	es = opt_local_expr(plist_get(in->os, 0));
	for (i = 0; i < es->size; i++) {
		e = plist_get(es, i);
		gen_local_expr_linux(g, e);
	}
}

void gen_tuple_of(Gg, struct LocalExpr *e) {
	if (!e->tuple)
		return;
	u32 i, j;
	struct PList *es;

	g->indent_level++;
	for (i = 0; i < e->tuple->size; i++) {
		es = opt_local_expr(plist_get(e->tuple, i));

		for (j = 0; j < es->size; j++)
			gen_local_expr_linux(g, plist_get(es, j));
	}
	g->indent_level--;
}

void merge_tuple_of_to(struct LocalExpr *of, struct LocalExpr *to) {
	if (!of->tuple)
		return;
	if (!to->tuple) {
		to->tuple = of->tuple;
		return;
	}
	plat_plist(to->tuple, of->tuple);
}

void gen_real(struct Gner *g, struct LocalExpr *e) {}

void gen_var(struct Gner *g, struct LocalExpr *e) {}

void var_(struct Gner *g, let_lvar_gvar) {
	if (lvar) {
		blat_ft(size_str(lvar->lvar_size)); // *байт
		sprint_ft(PAR_RBP);					// (рбп
		blat_ft(lvar->name->view);			// перем
		sprint_ft(R_PAR);					// )
	} else {								// gvar
		blat_ft(size_str(gvar->gvar_size)); // *байт
		sprint_ft(L_PAR);					// (
		blat_ft(gvar->signature);			// сигнатура
		sprint_ft(R_PAR);					// )
	}
}
void sib(struct Gner *g, uc size, enum RegCode base, uc scale,
		 enum RegCode index, long disp, uc is_disp_blist) {

	blat_fun_text(size_str(size));
	ft_add('(');
	if (base) {
		reg_(base);
	}
	if (scale > 1) {
		int_add(g->fun_text, scale);
		ft_add(' ');
		if (!index)
			exit(166);
		goto write_index;
	} else if (index) {
	write_index:
		blat_ft(just_get_reg(g->cpu, index)->name);
		if (disp)
			ft_add(' ');
	}
	if (disp) {
		if (is_disp_blist)
			blat_ft((struct BList *)disp);
		else
			int_add(g->fun_text, disp);
	}
	ft_add(')');
}
void mov_var_(Gg, let_lvar_gvar) {
	iprint_fun_text(SA_MOV);
	var_(g, lvar, gvar);
}
void mov_reg_(Gg, enum RegCode reg) {
	iprint_fun_text(SA_MOV);
	reg_(reg);
}
void mov_reg_var(Gg, enum RegCode reg, let_lvar_gvar) {
	mov_reg_(g, reg);
	var_enter(lvar, gvar);
}

int le_depth(struct LocalExpr *e) {
	int l_depth, r_depth;

	return is_primary(e)			  ? 1
		   : is_unary(e) || lce(BOOL) ? 2
		   : is_bin_le(e)
			   ? 1 + (l_depth = le_depth(e->l), r_depth = le_depth(e->r),
					  l_depth > r_depth ? l_depth : r_depth)
		   // : lcea(CALL) ? cuz side effect
		   // : lcea(INC) ?  cuz side effect
		   // : lcea(DEC) ?  cuz side effect
		   : lcea(INDEX)		? le_depth(e->l) + le_depth(e->r)
		   : lcea(FIELD_OF_PTR) ? le_depth(e->l) + 1
		   : lcea(FIELD)		? le_depth(e->l) + 1
								: (exit(151), -1);
}

// LE_PRIMARY_VAR = 3, LE_AFTER_INDEX = 36, LE_AFTER_FIELD = 41,

// TODO: mem is:
// * * local var, arr(not ptr), field(not ptr)
// * * global var, arr(not ptr), field(not ptr)

#define index_of_int                                                           \
	((lcea(INDEX) && e->l->type->code == TC_ARR && is_mem(e->l) &&             \
	  lceep(e->r, INT)))

int is_mem(struct LocalExpr *e) {
	return lcep(VAR) || (lcea(FIELD) && is_mem(e->l)) || index_of_int;
}

void inner_mem(Gg, struct LocalExpr *e) {
	if (lcep(VAR)) {
		declare_lvar_gvar;
		get_assignee_size(g, e, &gvar, &lvar);
		if (lvar) {
			reg_(R_RBP);
			blat_ft(lvar->name->view);
		} else {
			blat_ft(gvar->signature);
		}

	} else if (lcea(INDEX)) {
		struct TypeExpr *item_type = arr_type(e->l->type);
		int item_size = unsafe_size_of_type(item_type);

		inner_mem(g, e->l);
		// its may be used out of context, so need to check if index is INT
		if (lceep(e->r, INT) && e->r->tvar->num && item_size) {
			sprint_ft(MEM_PLUS);
			int_add(g->fun_text, e->r->tvar->num * item_size);
		}
	} else { // FIELD
		struct BList *field_full_name = (struct BList *)(long)e->tvar->real;

		inner_mem(g, e->l);
		sprint_ft(MEM_PLUS);
		blat_ft(field_full_name);
	}
}

void mem_(Gg, struct LocalExpr *e, int of_size) {
	of_size ? blat_ft(size_str(of_size))
			: blat_ft(size_str(unsafe_size_of_type(e->type)));
	ft_add('(');
	inner_mem(g, e);
	ft_add(')'), ft_add(' ');
}

void gen_mem_tuple(Gg, struct LocalExpr *e) {
	gen_tuple_of(g, e);

	if (lcea(FIELD)) {
		gen_mem_tuple(g, e->l);
	} else if (lcea(INDEX)) {
		gen_mem_tuple(g, e->l);
		gen_mem_tuple(g, e->r);
	}
}

/*
x = e
x can be:
* * var...-@...[literal]...field or none is mem

* * e[index]
* * e -> or -@ field
* * (*e)

also x can be trailed by
-@ field
[literal]
*/

struct LocalExpr *find_trailed(struct LocalExpr *e) {
	return lcea(FIELD) || index_of_int ? find_trailed(e->l) : e;
}
struct LocalExpr *is_not_assignable_or_trailed(struct LocalExpr *e) {
	return lcea(FIELD) || index_of_int ? find_trailed(e->l)
		   : lcea(FIELD_OF_PTR) || lceu(ADDR) || lcea(INDEX) ? e
															 : 0;
}

// TODO: here may need to do tuple gen in imt but later
// imt - inner mem text
#define blat_imt(l) (blat_blist(imt, (l)))
#define sprint_imt(str) (blat(imt, (uc *)(SA_##str), (SA_##str##_LEN - 1)))
#define imt_reg(rc) (blat_imt(just_get_reg(g->cpu, (rc))->name))
#define imt_add(b) (blist_add(imt, (b)))

struct Reg *begin_last_inner_mem(Gg, struct LocalExpr *e, struct BList *imt) {
	struct Reg *r1 = 0, *r2 = 0;
	struct LocalExpr *left = e->l, *trailed;

	if (lceu(ADDR)) {
		r1 = gen_to_reg(g, left, QWORD);
		if (r1->size != QWORD)
			exit(173);
		imt_reg(r1->reg_code);
	} else if (lcea(INDEX)) {
		struct TypeExpr *iant_type = left->type;
		int item_size = iant_type->code == TC_PTR
							? unsafe_size_of_type(ptr_targ(iant_type))
							: unsafe_size_of_type(arr_type(iant_type));
		struct BList *size_str;

		if (lceep(e->r, INT)) {
			if (iant_type->code == TC_PTR) {
				size_str = int_to_str(item_size * e->r->tvar->num);
				r1 = gen_to_reg(g, left, QWORD);

				imt_reg(r1->reg_code), imt_add(' ');
				blat_imt(size_str);

			} else if (iant_type->code == TC_ARR) {
				size_str = int_to_str(item_size * e->r->tvar->num);

				if ((trailed = is_not_assignable_or_trailed(left))) {
					r1 = last_inner_mem(g, left, trailed, imt);

					if (item_size && e->r->tvar->num)
						sprint_imt(MEM_PLUS), blat_imt(size_str);
				} else
					exit(179);
			}
		} else {
			r2 = gen_to_reg(g, e->r, QWORD); // index

			if (iant_type->code == TC_PTR) {
				size_str = int_to_str(item_size);
				r1 = gen_to_reg(g, left, QWORD);

				imt_reg(r1->reg_code), imt_add(' ');
				blat_imt(size_str), imt_add(' '), imt_reg(r2->reg_code);

			} else if (iant_type->code == TC_ARR) {
				size_str = int_to_str(item_size);

				if (is_mem(left)) {
					if (is_in_word(item_size)) {
						imt_add('0' + item_size), imt_add(' ');
					} else {
						op_reg_(IMUL, r2->reg_code);
						reg_(r2->reg_code);
						add_int_with_hex_comm(fun_text, item_size);
					}
					imt_reg(r2->rf->r->reg_code), imt_add(' ');

					exch(imt, g->fun_text, g->tmp_blist);
					inner_mem(g, left);
					exch(imt, g->fun_text, g->tmp_blist);

					r1 = r2, r2 = 0;
				} else if ((trailed = is_not_assignable_or_trailed(left))) {
					op_reg_(IMUL, r2->reg_code);
					reg_(r2->reg_code);
					add_int_with_hex_comm(fun_text, item_size);

					r1 = last_inner_mem(g, left, trailed, imt);
					op_reg_reg(ADD, r1, r2);
				} else
					exit(176);
			}
		}

		blist_clear_free(size_str);

	} else if (lcea(FIELD_OF_PTR)) {
		struct BList *field_full_name = (struct BList *)(long)e->tvar->real;
		r1 = gen_to_reg(g, left, 0);

		imt_reg(r1->reg_code), imt_add(' '), blat_imt(field_full_name);
		blist_clear_free(field_full_name);
	}

	free_reg_rf_if_not_zero(r2);
	return r1;
}

void trail_last_inner_mem(Gg, struct LocalExpr *e, struct LocalExpr *trailed,
						  struct BList *imt) {
	if (e == trailed)
		return;
	trail_last_inner_mem(g, e->l, trailed, imt);

	if (lcea(INDEX)) {
		struct TypeExpr *item_type = arr_type(e->l->type);
		int item_size = unsafe_size_of_type(item_type);

		if (lceep(e->r, INT) && e->r->tvar->num && item_size) {
			sprint_imt(MEM_PLUS);
			int_add(imt, e->r->tvar->num * item_size);
		}
	} else if (lcea(FIELD)) {
		struct BList *field_full_name = (struct BList *)(long)e->tvar->real;

		sprint_imt(MEM_PLUS), blat_imt(field_full_name);
		blist_clear_free(field_full_name);
	} else
		exit(123);
}

struct Reg *last_inner_mem(Gg, struct LocalExpr *e, struct LocalExpr *trailed,
						   struct BList *imt) {
	struct Reg *r = begin_last_inner_mem(g, trailed, imt);
	if (e != trailed)
		trail_last_inner_mem(g, e, trailed, imt);
	return r;
}

struct Reg *gen_to_reg_with_last_mem(Gg, struct LocalExpr *e,
									 struct LocalExpr *trailed,
									 struct BList **last_mem_str) {
	struct BList *imt = !(*last_mem_str) ? new_blist(64) : *last_mem_str;
	struct Reg *r = 0;

	blat_imt(size_str(unsafe_size_of_type(e->type)));
	imt_add('(');
	r = last_inner_mem(g, e, trailed, imt);
	imt_add(')'), imt_add(' ');

	*last_mem_str = imt;
	return r;
}
