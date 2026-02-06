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

void gen_if(struct Gner *g, struct LocalExpr *e);
void gen_if_elif(struct Gner *g, struct LocalExpr *e);
void gen_if_else(struct Gner *g, struct LocalExpr *e);
void gen_range_loop(struct Gner *g, struct LocalExpr *e);
void gen_then_loop(Gg, struct LocalExpr *loop);

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
	return;

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
		print_le(e->co.cond, 0);
		printf(" ? ");
		print_le(e->l, 0);
		printf(" : ");
		print_le(e->r, 0);
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
	} else if (is_if(e)) {
		printf("?? ");
		print_le(e->co.cond, 0);
	} else if (lce(RANGE_LOOP)) {
		print_le((void *)e->tvar->num, 0);
		printf(" => %s(%s", take_color_level(), COLOR_RESET);
		print_le(e->l, 0);
		(e->flags & DDD) ? printf(" ... ") : printf(" ..= ");
		print_le(e->r, 0);
		if (e->co.cond) {
			printf(" шаг ");
			print_le(e->co.cond, 0);
		}
		if (e->flags & LOOP_IS_BACKWARD)
			printf(" назад");
		printf("%s)%s ( ... )", remove_color_level(), COLOR_RESET);
	} else if (lce(AS)) {
		struct BList *other = zero_term_blist(type_to_blist_from_str(e->type));
		printf("%s(%sокак %s ", take_color_level(), COLOR_RESET, bs(other));
		blist_clear_free(other);
		print_le(e->l, 0);
		printf("%s)%s", remove_color_level(), COLOR_RESET);
	} else if (lce(DECLARE_VAR)) {
		struct BList *other = zero_term_blist(type_to_blist_from_str(e->type));
		printf("%s(%s", take_color_level(), COLOR_RESET);
		print_le(e->l, 0);
		printf(" есть ");
		printf("%s%s)%s", bs(other), remove_color_level(), COLOR_RESET);
		blist_clear_free(other);
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
#define print_type(t)                                                          \
	other = zero_term_blist(type_to_blist_from_str((t)));                      \
	print_str(bs(other)), blist_clear_free(other);

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
		print_orher(e->co.cond);
		print_str(" ? ");
		print_orher(e->l);
		print_str(" : ");
		print_orher(e->r);
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
	} else if (is_if(e)) {
		print_str("?? ");
		print_orher(e->co.cond);
	} else if (lce(RANGE_LOOP)) {
		print_orher((void *)e->tvar->num);
		print_str(" => (");
		print_orher(e->l);
		(e->flags & DDD) ? print_str(" ... ") : print_str(" ..= ");
		print_orher(e->r);
		if (e->co.cond) {
			print_str(" шаг ");
			print_orher(e->co.cond);
		}
		if (e->flags & LOOP_IS_BACKWARD)
			print_str(" назад");
		print_str(") ( ... )");
	} else if (lce(AS)) {
		print_str("(окак ");
		print_type(e->type);
		print_str(" ");
		print_orher(e->l);
		print_str(")");
	} else if (lce(DECLARE_VAR)) {
		print_str("(");
		print_orher(e->l);
		print_str(" есть ");
		print_type(e->type);
		print_str(")");
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
	indent_line(g, g->fun_text), ft_add(';'), ft_add(' ');
	struct BList *expr_view = zero_term_blist(bprint_le(e, 1));
	blat_fun_text(expr_view);
	blist_clear_free(expr_view);
	print_le(e, 1);

	gen_tuple_of(g, e);

	if (lceb(ASSIGN))
		gen_assign(g, e);
	else if (lceu(INC) || lcea(INC) || lceu(DEC) || lcea(DEC))
		gen_dec_inc(g, e->l, lceu(INC) || lcea(INC));
	else if (lcea(CALL))
		gen_call(g, e);
	else if (lceb(TERRY) && causes_more_than_just_gvar(e))
		gen_terry(g, e);
	else if (lce(IF_ELSE))
		gen_if_else(g, e);
	else if (lce(IF_ELIF))
		gen_if_elif(g, e);
	else if (lce(IF))
		gen_if(g, e);
	else if (lce(RANGE_LOOP))
		gen_range_loop(g, e);
	else if (lce(THEN_LOOP))
		gen_then_loop(g, e);
	else {
		// 	exit(159);
		printf("### NOT GEN LOCAL EXPR INFO: e->code == %d\n", e->code);
		print_le(e, 1);
	}
}

void gen_opted(Gg, struct LocalExpr *e) {
	struct PList *es = opt_local_expr(e);
	for (u32 i = 0; i < es->size; i++)
		gen_local_expr_linux(g, plist_get(es, i));
}

void gen_tuple_of(Gg, struct LocalExpr *e) {
	if (!e->tuple)
		return;
	g->indent_level++;
	for (u32 i = 0; i < e->tuple->size; i++)
		gen_opted(g, plist_get(e->tuple, i));
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

void sib(struct Gner *g, uc size, enum RegCode base, uc scale,
		 enum RegCode index, long disp, uc is_disp_blist) {

	blat_fun_text(size_str(size));
	ft_add('(');
	if (base) {
		reg_rc_(base);
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

int le_depth(struct LocalExpr *e) {
	int l_depth, r_depth;

	return is_primary(e)						 ? 1
		   : is_unary(e) || lce(BOOL) || lce(AS) ? 2
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

#define index_of_int                                                           \
	((lcea(INDEX) && e->l->type->code == TC_ARR && is_mem(e->l) &&             \
	  lceep(e->r, INT)))

int is_mem(struct LocalExpr *e) {
	return lcep(VAR) || lce(DECLARE_VAR) || (lcea(FIELD) && is_mem(e->l)) ||
		   index_of_int;
}

void inner_mem(Gg, struct LocalExpr *e) {
	declare_lvar_gvar;

	if (lcep(VAR)) {
	eval_var:
		get_assignee_size(g, e, &gvar, &lvar);
		if (lvar) {
			reg_rc_(R_RBP);
			blat_ft(lvar->name->view);
		} else {
			blat_ft(gvar->signature);
		}
	} else if (lce(DECLARE_VAR)) {
		// declare var
		delcare_var_from_LE_DECLARE_VAR(g, e);
		// eval var
		goto eval_var;

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
#define imt_reg_(rc) (imt_reg((rc)), imt_add(' '))
#define imt_add(b) (blist_add(imt, (b)))

struct Reg *begin_last_inner_mem(Gg, struct LocalExpr *e, struct BList *imt) {
	struct Reg *r1 = 0, *r2 = 0;
	struct LocalExpr *left = e->l, *trailed;

	if (lceu(ADDR)) {
		r1 = gen_to_reg(g, left, QWORD);
		if (r1->size != QWORD)
			exit(173);
		imt_reg_(r1->reg_code);
		imt_add('0'); // need zero to parry '+' if will be after r1
	} else if (lcea(INDEX)) {
		struct TypeExpr *iant_type = left->type;
		int item_size = iant_type->code == TC_PTR
							? unsafe_size_of_type(ptr_targ(iant_type))
							: unsafe_size_of_type(arr_type(iant_type));

		if (lceep(e->r, INT)) {
			struct BList *size_str = int_to_str(item_size * e->r->tvar->num);

			if (iant_type->code == TC_PTR) {
				r1 = gen_to_reg(g, left, QWORD);
				imt_reg_(r1->reg_code);
				blat_imt(size_str);

			} else if (iant_type->code == TC_ARR) {
				if ((trailed = is_not_assignable_or_trailed(left))) {
					r1 = last_inner_mem(g, left, trailed, imt);

					if (item_size && e->r->tvar->num)
						sprint_imt(MEM_PLUS), blat_imt(size_str);
				} else
					exit(179);
			}
			blist_clear_free(size_str);
		} else {
			r2 = gen_to_reg(g, e->r, QWORD); // index

			if (iant_type->code == TC_PTR) {
				mul_on_int(g, r2, item_size);
				r2 = get_reg_to_size(g, r2, QWORD);
				r1 = gen_to_reg(g, left, QWORD);

				op_reg_reg(ADD, r1, r2);
				imt_reg_(r1->reg_code);
				imt_add('0'); // need zero to parry '+' if will be after r1

			} else if (iant_type->code == TC_ARR) {
				if (is_mem(left)) {
					if (is_in_word(item_size))
						imt_add('0' + item_size), imt_add(' ');
					else
						mul_on_int(g, r2, item_size);
					imt_reg_(r2->rf->r->reg_code);

					exch(imt, g->fun_text, g->tmp_blist);
					inner_mem(g, left);
					exch(imt, g->fun_text, g->tmp_blist);

					r1 = r2, r2 = 0;
				} else if ((trailed = is_not_assignable_or_trailed(left))) {
					mul_on_int(g, r2, item_size);

					r1 = last_inner_mem(g, left, trailed, imt);
					op_reg_reg(ADD, r1, r2);
				} else
					exit(176);
			}
		}

	} else if (lcea(FIELD_OF_PTR)) {
		struct BList *field_full_name = (struct BList *)(long)e->tvar->real;
		r1 = gen_to_reg(g, left, QWORD);

		imt_reg_(r1->reg_code), blat_imt(field_full_name);
		// blist_clear_free(field_full_name);
	}

	free_register(r2);
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
		// blist_clear_free(field_full_name);
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

int lm_size = 0;
struct Reg *gen_to_reg_with_last_mem(Gg, struct LocalExpr *e,
									 struct LocalExpr *trailed,
									 struct BList **last_mem_str) {
	struct BList *imt = !(*last_mem_str) ? new_blist(64) : *last_mem_str;
	struct Reg *r = 0;

	lm_size ? blat_imt(size_str(lm_size))
			: blat_imt(size_str(unsafe_size_of_type(e->type)));
	imt_add('(');
	r = last_inner_mem(g, e, trailed, imt);
	imt_add(')'), imt_add(' ');

	*last_mem_str = imt;
	lm_size = 0;
	return r;
}
