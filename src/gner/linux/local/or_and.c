#include "../../gner.h"

#define cbe(bin) (le == LE_BIN_##bin)
#define str_len_be(code) ((str = SA_##code, len = SA_##code##_LEN))

void iprint_jmp(Gg, enum LE_Code le, int is_u) {
	const char *str;
	u32 len;
	if (is_u) {
		cbe(LESS)		  ? str_len_be(JB)
		: cbe(LESSE)	  ? str_len_be(JBE)
		: cbe(MORE)		  ? str_len_be(JA)
		: cbe(MOREE)	  ? str_len_be(JAE)
		: cbe(EQUALS)	  ? str_len_be(JE)
		: cbe(NOT_EQUALS) ? str_len_be(JNE)
						  : exit(133);
	} else {
		cbe(LESS)		  ? str_len_be(JL)
		: cbe(LESSE)	  ? str_len_be(JLE)
		: cbe(MORE)		  ? str_len_be(JG)
		: cbe(MOREE)	  ? str_len_be(JGE)
		: cbe(EQUALS)	  ? str_len_be(JE)
		: cbe(NOT_EQUALS) ? str_len_be(JNE)
						  : exit(134);
	}
	indent_line(g, g->fun_text);
	blat(g->fun_text, (uc *)str, len - 1);
}

void and_cmp(Gg, struct LocalExpr *e, struct BList *false_label) {
	struct Reg *r1;
	struct BList *this_end_label;

	if (is_uses_cmp(e)) {
		just_cmp(g, e);

		iprint_jmp(g, opposite_cmp_le(e->code), is_u_type(e->l->type->code));
		blat_ft(false_label), ft_add('\n');
	} else if (lceb(AND)) {
		and_cmp(g, e->l, false_label);
		and_cmp(g, e->r, false_label);
	} else if (lceb(OR)) {
		this_end_label = take_label(g, LC_ELSE);

		or_cmp(g, e->l, this_end_label);
		and_cmp(g, e->r, false_label);
		add_label(this_end_label);
		blist_clear_free(this_end_label);
	} else {
		r1 = gen_to_reg(g, e, 0);
		op_reg_reg(TEST, r1, r1);
		free_reg_family(r1->rf);

		op_(J0);
		blat_ft(false_label), ft_add('\n');
	}
}

struct Reg *and_to_reg(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0;

	struct BList *exit_label, *false_label;

	if (is_num_le(e->l) || is_num_le(e->r))
		exit(121);

	false_label = take_label(g, LC_ELSE);
	exit_label = take_label(g, LC_ELSE);

	and_cmp(g, e->l, false_label);
	and_cmp(g, e->r, false_label);

	// mov reg, 1
	r1 = try_borrow_reg(e->tvar, g, unsafe_size_of_type(e->type));
	op_reg_(MOV, r1->reg_code);
	add_int_with_hex_comm(fun_text, 1);
	// jmp exit
	op_(JMP);
	blat_ft(exit_label), ft_add('\n');

	// false_label:
	add_label(false_label);
	// mov reg, 0
	op_reg_reg(XOR, r1, r1);
	// exit_label:
	add_label(exit_label);

	if (!r1)
		exit(122);
	blist_clear_free(exit_label);
	blist_clear_free(false_label);

	return r1;
}

void or_cmp(Gg, struct LocalExpr *e, struct BList *true_label) {
	struct Reg *r1;
	struct BList *this_end_label;

	if (is_uses_cmp(e)) {
		just_cmp(g, e);

		iprint_jmp(g, e->code, is_u_type(e->l->type->code));
		blat_ft(true_label), ft_add('\n');
	} else if (lceb(OR)) {
		or_cmp(g, e->l, true_label);
		or_cmp(g, e->r, true_label);
	} else if (lceb(AND)) {
		this_end_label = take_label(g, LC_ELSE);

		and_cmp(g, e->l, this_end_label);
		or_cmp(g, e->r, true_label);
		add_label(this_end_label);
		blist_clear_free(this_end_label);
	} else {
		if (is_mem(e)) {
			op_mem_(CMP, e, 0);
		} else {
			r1 = gen_to_reg(g, e, 0);
			op_reg_(CMP, r1->reg_code);
			free_reg_family(r1->rf);
		}
		add_int_with_hex_comm(fun_text, 0);

		op_(JN0);
		blat_ft(true_label), ft_add('\n');
	}
}

struct Reg *or_to_reg(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0;

	struct BList *exit_label, *true_label, *false_label;

	if (is_num_le(e->l) || is_num_le(e->r))
		exit(121);

	true_label = take_label(g, LC_ELSE);
	false_label = take_label(g, LC_ELSE);
	exit_label = take_label(g, LC_ELSE);

	or_cmp(g, e->l, true_label);
	and_cmp(g, e->r, false_label);

	// true_label:
	add_label(true_label);
	// mov reg, 1
	r1 = try_borrow_reg(e->tvar, g, unsafe_size_of_type(e->type));
	op_reg_(MOV, r1->reg_code);
	add_int_with_hex_comm(fun_text, 1);
	// jmp exit
	op_(JMP);
	blat_ft(exit_label), ft_add('\n');
	// false_label:
	add_label(false_label);
	// mov reg, 0
	op_reg_reg(XOR, r1, r1);
	// exit_label:
	add_label(exit_label);

	if (!r1)
		exit(121);
	blist_clear_free(exit_label);
	blist_clear_free(true_label);
	blist_clear_free(false_label);

	return r1;
}
