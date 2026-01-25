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

#define shl_and_shr_or_sal_and_sar                                             \
	if (lceb(SHR)) {                                                           \
		if (is_unsigned) {                                                     \
			op_(SHR);                                                          \
		} else {                                                               \
			op_(SAR);                                                          \
		}                                                                      \
	} else {                                                                   \
		if (is_unsigned) {                                                     \
			op_(SHL);                                                          \
		} else {                                                               \
			op_(SAL);                                                          \
		}                                                                      \
	}

struct Reg *shift_on_int(Gg, struct LocalExpr *e, struct Reg *r1) {
	long shift_on = e->r->tvar->num;
	int is_unsigned = is_u_type(e->type->code);
	struct Reg *r2;

	if (shift_on > 63 || shift_on < 0) {
		r2 = prime_to_reg(g, e->r, r1->size);
		return shift_on_reg(g, e, r1, r2);
	}
	if (shift_on == 1) {
		if (lceb(SHR)) {
			if (is_unsigned) {
				op_(SHR1);
			} else {
				op_(SAR1);
			}
		} else {
			if (is_unsigned) {
				op_(SHL1);
			} else {
				op_(SAL1);
			}
		}
		reg_enter(r1);
	} else {
		shl_and_shr_or_sal_and_sar;
		reg_enter(r1);
		add_int_with_hex_comm(fun_text, shift_on);
	}
	return r1;
}

struct Reg *shift_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
						 struct Reg *r2) {
	int is_unsigned = is_u_type(e->l->type->code);

	get_reg_to_rf(e->tvar, g, r2, g->cpu->c);

	shl_and_shr_or_sal_and_sar;
	reg_enter(r1);

	free_register(r2);
	return r1;
}

struct Reg *mul_on_int(Gg, struct Reg *r1, long mul_on) {
	if (mul_on == 1)
		return r1;
	if (is_pow_of_two(mul_on)) {
		mul_on = find_pow_of_2(mul_on);
		if (mul_on == 1) {
			op_reg_enter(SAL1, r1);
		} else {
			op_reg_(SAL, r1);
			add_int_with_hex_comm(fun_text, mul_on);
		}
	} else {
		op_reg_(IMUL, r1);
		reg_(r1);
		add_int_with_hex_comm(fun_text, mul_on);
	}
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
				op_reg_enter(SHR1, r1);
			} else {
				op_reg_(SHR, r1);
				add_int_with_hex_comm(fun_text, pow);
			}
		} else {
			r2 = try_borrow_reg(e->tvar, g, r1->size);

			op_reg_(LEA, r2->rf->r);
			sib(g, QWORD, 0, 0, r1->rf->r->reg_code, div_on - 1, 0),
				ft_add('\n');

			op_reg_reg(TEST, r1, r1);
			op_reg_reg(CMOVS, r1, r2);

			if (div_on == 1) {
				op_reg_enter(SAR1, r1);
			} else {
				op_reg_(SAR, r1);
				add_int_with_hex_comm(fun_text, pow);
			}
		}
		if (need_neg) {
			op_reg_enter(NEG, r1);
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
		// 		op_reg_(SAR, r1);
		// 		add_int_with_hex_comm(fun_text, k - 1);
		// 		// a * m
		// 		op_reg_(IMUL, r1);
		// 		reg_(r1);
		// 		add_int_with_hex_comm(fun_text, m);
		// 		// (a * m) >> k
		// 		op_reg_(SAR, r1);
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

	free_register(r2);
	return r1;
}

struct Reg *mod_on_int(Gg, struct LocalExpr *e, struct Reg *r1) {
	struct Reg *r2 = prime_to_reg(g, e->r, r1->size);
	return mod_on_reg(g, e, r1, r2);
}

void divide_on_mem(Gg, struct LocalExpr *e, struct Reg **to_rAX,
				   struct Reg **to_rDX) {
	struct Reg *rDX = 0, *r1 = *to_rAX;

	get_reg_to_rf(e->tvar, g, r1, g->cpu->a);

	if (g->cpu->d->r->allocated) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, g->cpu->d, rDX->rf, DO_MOV); // rDX is rDX
	} else {
		rDX = try_alloc_reg(e->tvar, g->cpu->d, r1->size);
	}

	if (r1->size == WORD)
		op_(CWD);
	else if (r1->size == DWORD)
		op_(CDQ);
	else if (r1->size == QWORD)
		op_(CQO);
	op_(IDIV);
	mem_enter(e->r, 0);

	// return
	*to_rAX = r1;
	*to_rDX = rDX;
}

struct Reg *div_on_mem(Gg, struct LocalExpr *e, struct Reg *r1) {
	struct Reg *rDX;
	divide_on_mem(g, e, &r1, &rDX);
	free_register(rDX);
	return r1;
}

struct Reg *mod_on_mem(Gg, struct LocalExpr *e, struct Reg *r1) {
	struct Reg *rDX;
	divide_on_mem(g, e, &r1, &rDX);
	free_register(r1);
	return rDX;
}

void divide_on_reg(Gg, struct LocalExpr *e, struct Reg **to_rAX,
				   struct Reg **to_rDX) {
	struct Reg *rDX = 0, *r1 = *to_rAX, *r2 = *to_rDX;

	get_reg_to_rf(e->tvar, g, r1, g->cpu->a);

	if (is_rDX(r2->reg_code)) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, r2->rf, rDX->rf, DO_MOV); // r2 became not rDX
	} else if (g->cpu->d->r->allocated) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, g->cpu->d, rDX->rf, DO_MOV); // rDX is rDX
	} else {
		rDX = try_alloc_reg(e->tvar, g->cpu->d, r2->size);
	}

	if (r1->size != r2->size)
		exit(111);
	if (r1->size == WORD)
		op_(CWD);
	else if (r1->size == DWORD)
		op_(CDQ);
	else if (r1->size == QWORD)
		op_(CQO);
	op_reg_enter(IDIV, r2);

	if (r2->rf != g->cpu->d)
		free_register(r2);
	// return
	*to_rAX = r1;
	*to_rDX = rDX;
}

struct Reg *div_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
					   struct Reg *r2) {
	divide_on_reg(g, e, &r1, &r2);
	free_register(r2);
	return r1;
}

struct Reg *mod_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
					   struct Reg *r2) {
	divide_on_reg(g, e, &r1, &r2);
	free_register(r1);
	return r2;
}
