#include "../../gner.h"

void and_cmp(Gg, struct LocalExpr *e, struct BList *false_label) {
	struct Reg *r1;

	if (is_uses_cmp(e)) {
		just_cmp(g, e);
	} else if (lceb(AND)) {
		and_cmp(g, e->l, false_label);
		and_cmp(g, e->r, false_label);
	} else {
		if (lceep(e, VAR)) {
			declare_lvar_gvar;
			get_assignee_size(g, e, &gvar, &lvar);

			isprint_ft(CMP);
			var_(g, lvar, gvar);
		} else {
			r1 = gen_to_reg(g, e, 0);
			op_reg_(CMP, r1->reg_code);
			free_reg_family(r1->rf);
		}
		add_int_with_hex_comm(fun_text, 0);

		isprint_ft(J0);
		blat_ft(false_label), ft_add('\n');
	}
}

struct Reg *and_to_reg(Gg, struct LocalExpr *e, int reg_size,
					   struct BList *false_label) {
	struct Reg *r1 = 0;
	reg_size = unsafe_size_of_type(e->type);

	struct BList *exit_label;
	int to_gen_labels = false_label == 0;

	if (is_num_le(e->l) || is_num_le(e->r))
		exit(121);

	if (to_gen_labels) {
		false_label = take_label(g, LC_ELSE),
		exit_label = take_label(g, LC_ELSE);

		and_cmp(g, e->l, false_label);
		and_cmp(g, e->r, false_label);

		// mov reg, 1
		r1 = try_borrow_reg(e->tvar, g, reg_size);
		op_reg_(MOV, r1->reg_code);
		add_int_with_hex_comm(fun_text, 1);
		// jmp exit
		isprint_ft(JMP);
		blat_ft(exit_label), ft_add('\n');

		// false_label:
		add_label(false_label);
		// mov reg, 0
		op_reg_reg(XOR, r1, r1);
		// exit_label:
		add_label(exit_label);
	} else {
		exit(127);
		and_cmp(g, e->l, false_label);
		and_cmp(g, e->r, false_label);
	}

	if (!r1 && to_gen_labels)
		exit(122);
	return r1;
}

struct Reg *or_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct LocalExpr *l = e->l, *r = e->r;
	struct Reg *r1 = 0;

	if (is_num_le(l) || is_num_le(r))
		exit(124);
	if (!r1)
		exit(123);
	return r1;
}
