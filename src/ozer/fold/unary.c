#include "../../gner/gner.h"

cant_on_reals(BIT_NOT, "~");
cant_on_nums(INC, "++");
cant_on_nums(DEC, "--");
cant_on_nums(AMPER, "&");
cant_on_nums(ADDR, "*");

#define int_op(op) (num->tvar->num = op num->tvar->num)
#define real_op(op) (num->tvar->real = op num->tvar->real)
#define num_op(op)                                                             \
	do {                                                                       \
		if (is_INT_le(num))                                                    \
			int_op(op);                                                        \
		else                                                                   \
			real_op(op);                                                       \
	} while (0)

#define num_err(op) (eet(e->tvar, CANT_##op##_ON_NUMS, 0))
#define real_err(op) (eet(e->tvar, CANT_##op##_ON_REALS, 0))

// bool num -> num == 0 ? 0 : 1
void unary_or_bool_of_num(struct LocalExpr *e) {
	struct LocalExpr *num = e->l, *was_e;

	if (!is_num_le(num)) {
		opt_bin_constant_folding(e);
		return;
	}

	if (lce(BOOL)) {
		num->tvar->num = is_le_num(num, 0) ? 0 : 1;
		num->code = LE_PRIMARY_INT;
	} else if (lceu(MINUS))
		num_op(-);
	else if (lceu(INC))
		num_err(INC);
	else if (lceu(DEC))
		num_err(DEC);
	else if (lceu(NOT))
		num_op(!);
	else if (lceu(BIT_NOT)) {
		if (is_INT_le(num))
			int_op(~);
		else
			real_err(BIT_NOT);
	} else if (lceu(AMPER))
		num_err(AMPER);
	else if (lceu(ADDR))
		num_err(ADDR);

	if (is_INT_le(num))
		update_int_view(num);
	else
		update_real_view(num);

	merge_tuple_of_to(e, num);
	// save num
	was_e = num;
	// paste num, so that e is equal to num, but its 2 mems
	paste_le(e, num);
	// free num mem
	free(was_e);
}
