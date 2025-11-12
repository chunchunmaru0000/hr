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
		printf("%s", vs(e->tvar));
		print_le(e->l, 0);
	} else if (lcea(INC) || lcea(DEC)) {
		print_le(e->l, 0);
		printf("%s", vs(e->tvar));
	} else {
		printf("%s", vs(e->tvar));
	}

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
		print_tvar(e);
		print_orher(e->l);
	} else if (lcea(INC) || lcea(DEC)) {
		print_orher(e->l);
		print_tvar(e);
	} else {
		print_tvar(e);
	}

	if (with_n)
		blist_add(out, '\n');

	return out;
}

void gen_local_expression_linux(struct Gner *g, struct Inst *in) {
	struct LocalExpr *e;
	struct BList *expr_view;
	struct PList *es;
	u32 i;

	es = opt_local_expr(plist_get(in->os, 0));

	for (i = 0; i < es->size; i++) {
		e = plist_get(es, i);

		print_fun_text(SA_START_COMMENT);
		expr_view = zero_term_blist(bprint_le(e, 1));
		blat_fun_text(expr_view);
		blist_clear_free(expr_view);

		print_le(e, 1);

		free_all_regs(g->cpu);
		if (lceb(ASSIGN))
			gen_assign(g, e);
		else if (lceu(INC) || lcea(INC) || lceu(DEC) || lcea(DEC))
			gen_dec_inc(g, e->l, lceu(INC) || lcea(INC));
		// else if (lcea(CALL))
		// 	gen_call(g, e);
		// else if (lceb(TERRY))
		//	gen_terry(g, e);
		// else
		// 	exit(159);
		// printf("### GEN LOCAL EXPR INFO: e->code == %d\n", e->code);
	}
}

void gen_real(struct Gner *g, struct LocalExpr *e) {}

void gen_var(struct Gner *g, struct LocalExpr *e) {}

void var_(struct Gner *g, let_lvar_gvar) {
	if (lvar) {
		blat_fun_text(size_str(lvar->lvar_size)); // *байт
		print_fun_text(SA_PAR_RBP);				  // (рбп
		blat_fun_text(lvar->name->view);		  // перем
		blat_str_fun_text(SA_R_PAR);			  // )
	} else {									  // gvar
		blat_fun_text(size_str(gvar->gvar_size)); // *байт
		blat_str_fun_text(SA_L_PAR);			  // (
		blat_fun_text(gvar->signature);			  // сигнатура
		blat_str_fun_text(SA_R_PAR);			  // )
	}
}
void sib_(struct Gner *g, uc size, enum RegCode base, uc scale,
		  enum RegCode index, long disp, uc is_disp_blist) {

	blat_fun_text(size_str(size));
	sprint_ft(L_PAR); // (
	if (base) {
		blat_ft(just_get_reg(g->cpu, base)->name);
		ft_add(' ');
	}
	if (scale > 1) {
		int_add(g->fun_text, scale);
		ft_add(' ');
		if (!index)
			exit(166);
		blat_ft(just_get_reg(g->cpu, index)->name);
		ft_add(' ');
	} else if (index) {
		blat_ft(just_get_reg(g->cpu, index)->name);
		ft_add(' ');
	}
	if (disp) {
		if (is_disp_blist)
			blat_ft((struct BList *)disp);
		else
			int_add(g->fun_text, disp);
	}
	sprint_ft(R_PAR); // )
}
void mov_var_(Gg, let_lvar_gvar) {
	iprint_fun_text(SA_MOV);
	var_(g, lvar, gvar);
}
void mov_reg_(Gg, enum RegCode reg) {
	iprint_fun_text(SA_MOV);
	blat_ft(just_get_reg(g->cpu, reg)->name), ft_add(' ');
}
void mov_reg_var(Gg, enum RegCode reg, let_lvar_gvar) {
	mov_reg_(g, reg);
	var_(g, lvar, gvar);
	g->fun_text->size--, ft_add('\n');
}

void gen_assign(struct Gner *g, struct LocalExpr *e) {
	struct LocalExpr *assignee = e->l;
	struct LocalExpr *assignable = e->r;

	declare_lvar_gvar;
	struct TypeExpr *assignee_type = 0;
	uc assignee_size = 0;

	// printf("### GEN assignee INFO: assignee->code == %d\n", assignee->code);
	// printf("### GEN assignable INFO: assignable->code == %d\n",
	// 	   assignable->code);

	if (lceeb(assignee, ASSIGN)) {
	} else if (lceep(assignee, VAR)) {
		// assignee_size =
		get_assignee_size(g, assignee, &gvar, &lvar);
		assignee_type = lvar_gvar_type();

		compare_type_and_expr(assignee_type, assignable);

		if (lceep(assignable, INT) || lceep(assignable, REAL)) {
			mov_var_(g, lvar, gvar);

			if (assignable->code == LE_PRIMARY_INT) {
				add_int_with_hex_comm(fun_text, assignable->tvar->num);
			} else {
				real_add(g->fun_text, assignable->tvar->real); // число
				fun_text_add('\n');							   // \n
			}
		} else {
		}
	}

	// rax = try_borrow_reg(e->tvar, g->cpu, QWORD);
	//  reg = try_borrow_reg(e->tvar, g->cpu, );
	//   if (assignable->code == LE_BIN_ASSIGN) {
	//   	gen_to_reg(g, rax, assignable);
	//   	gen_to_reg(g, reg, assignable);
	//   }
}
