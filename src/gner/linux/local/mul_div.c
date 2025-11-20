#include "../../gner.h"
#include <stdio.h>

constr ZERO_DIVISION =
	"Деление на ноль невозможно ни в одном случае, потому что оно не имеет "
	"смысла. Для любого ненулевого числа a, уравнение a / 0 = x означает, "
	"что 0 * x = a, но ни одно число, умноженное на ноль, не даст a, "
	"поэтому такого решения не существует. В случае, если делить ноль на ноль "
	"0 / 0, любое число подходит как ответ, так как 0 * x = 0 "
	"выполняется для любого x, что делает результат неопределенным. ";

int find_pow_of_2(long value) {
	int shifts = 0;
	for (; value > 1; shifts++)
		value >>= 1;
	return shifts;
}

void get_reg_to_rax(struct Token *tvar, Gg, struct Reg *reg) {
	if (!is_rAX(reg->reg_code)) {
		if (g->cpu->a->r->allocated) {
			// reg is now points to rAX
			swap_basic_regs(g, g->cpu->a, reg->rf, DO_XCHG);
		} else {
			mov_reg_(g, R_RAX);
			reg_enter(reg->rf->r->reg_code);
			free_reg_family(reg->rf);
			reg = try_alloc_reg(tvar, g->cpu->a, reg->size);
		}
	}
}

struct Reg *mul_on_int(Gg, struct Reg *r1, struct LocalExpr *num) {
	long mul_on = num->tvar->num;

	if (is_pow_of_two(mul_on)) {
		mul_on = find_pow_of_2(mul_on);
		op_reg_(SAL, r1->reg_code);
	} else {
		op_reg_(IMUL, r1->reg_code);
		reg_(r1->reg_code);
	}
	add_int_with_hex_comm(fun_text, mul_on);

	return r1;
}

struct Reg *div_on_int(Gg, struct LocalExpr *e, struct Reg *r1) {
	long div_on = e->r->tvar->num, m;
	int need_neg = 0, is_unsigned = is_u_type(e->type->code), pow, k;
	struct Reg *r2 = 0;

	if (div_on == 0)
		eet(e->tvar, ZERO_DIVISION, 0);

	if (is_pow_of_two(div_on) ? (pow = find_pow_of_2(div_on))
		: div_on < 0 && is_pow_of_two(-div_on)
			? (div_on = -div_on, pow = find_pow_of_2(div_on), need_neg = 1)
			: 0) {

		if (is_unsigned) {
			if (div_on == 1) {
				op_reg_enter(SHR1, r1->reg_code);
			} else {
				op_reg_(SHR, r1->reg_code);
				add_int_with_hex_comm(fun_text, pow);
			}
		} else {
			r2 = try_borrow_reg(e->tvar, g, r1->size);

			op_reg_(LEA, r2->rf->r->reg_code);
			sib(g, QWORD, 0, 0, r1->rf->r->reg_code, div_on - 1, 0),
				ft_add('\n');

			op_reg_reg(TEST, r1, r1);
			op_reg_reg(CMOVS, r1, r2);

			if (div_on == 1) {
				op_reg_enter(SAR1, r1->reg_code);
			} else {
				op_reg_(SAR, r1->reg_code);
				add_int_with_hex_comm(fun_text, pow);
			}
		}
		if (need_neg) {
			op_reg_enter(NEG, r1->reg_code);
		}

	} else {
		// TODO: better div
		// 		// div = lambda a, d, k: (a * ((1 << k) // d + 1)) >> k
		// 		// m = ((1 << k) // d + 1)
		// 		// div = lambda a, d, k: (a * m) >> k
		// 		k = r1->size == QWORD ? 64 : 32;
		// 		m = ((1 << k) / div_on + 1);
		//
		// 		// ((a * m) >> k + 1) - (a >> k - 1)
		// 		r2 = try_borrow_reg(e->tvar, g, QWORD);
		// 		op_reg_reg(MOV, r2, r1);
		// 		// a >> k - 1
		// 		op_reg_(SAR, r1->reg_code);
		// 		add_int_with_hex_comm(fun_text, k - 1);
		// 		// a * m
		// 		op_reg_(IMUL, r1->reg_code);
		// 		reg_(r1->reg_code);
		// 		add_int_with_hex_comm(fun_text, m);
		// 		// (a * m) >> k
		// 		op_reg_(SAR, r1->reg_code);
		// 		add_int_with_hex_comm(fun_text, k + 1);
		// 		// -
		//
		//
		// 		if (!is_unsigned) {
		// 			// idiv = fn a,d,k: div(a,d,k) if a > 0 else div(a,d,k) + 1
		// 		}
		r2 = prime_to_reg(g, e->r, r1->size);
		return div_on_reg(g, e, r1, r2);
	}

	if (r2)
		free_reg_family(r2->rf);
	return r1;
}

struct Reg *div_on_mem(Gg, struct LocalExpr *e, struct Reg *r1) {
	struct Reg *rDX = 0;

	declare_lvar_gvar;
	get_assignee_size(g, e->r, &gvar, &lvar);

	get_reg_to_rax(e->tvar, g, r1);

	if (g->cpu->d->r->allocated) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, g->cpu->d, rDX->rf, DO_MOV); // rDX is rDX
	} else {
		rDX = try_alloc_reg(e->tvar, g->cpu->d, r1->size);
	}
	// CBW
	// CWDE
	// CDQE
	isprint_ft(IDIV);
	var_enter(lvar, gvar);
	// CWD
	// CDQ
	// CQO
	free_reg_family(rDX->rf);
	return r1;
}

struct Reg *div_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
					   struct Reg *r2) {
	struct Reg *rDX = 0;

	get_reg_to_rax(e->tvar, g, r1);

	if (is_rDX(r2->reg_code)) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, r2->rf, rDX->rf, DO_MOV); // r2 became not rDX
	} else if (g->cpu->d->r->allocated) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, g->cpu->d, rDX->rf, DO_MOV); // rDX is rDX
	} else {
		rDX = try_alloc_reg(e->tvar, g->cpu->d, r2->size);
	}

	// TODO: proper size
	// CBW
	// CWDE
	// CDQE
	isprint_ft(IDIV);
	reg_enter(r2->reg_code);
	// CWD
	// CDQ
	// CQO

	free_reg_family(r2->rf);
	free_reg_family(rDX->rf);
	return r1;
}
