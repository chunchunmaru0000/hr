#include "../pser.h"
#include <stdio.h>

constr FIELD_NAME_CAN_BE_ONLY_ID =
	"Поле по сути аргумент, а значит его имя может представлять лишь имя.";
constr WRONG_DOT_USAGE = "Неправильное использование '.' точки.";
constr HOW_TO_USE_DOT =
	"В С точка используется для обращения к полю разыменованной структуры, "
	"здесь для этого используется стрелка '-.',\nточка же используется в двух "
	"случаях:\n"
	" * для обращения к элементу счета: СЧЕТ.ЭЛЕМЕНТ\n "
	"* для передачи выражения в качестве первого аргумента при вызове "
	"функции:\n   проц.освободить(регистр), "
	"что будет равносильно освободить(проц, регистр)";
constr EXPECTED_VAR_EXPRESSION_TO_USE_WITH_VERB_IS =
	"При использовании слова 'есть' требуется использование выражения "
	"переменной слева от данного слова.";

struct LocalExpr *new_local_expr(enum LE_Code le_code, struct TypeExpr *type,
								 struct Token *tvar) {
	struct LocalExpr *e = malloc(sizeof(struct LocalExpr));
	e->code = le_code;
	e->flags = 0;
	e->type = type;
	e->tvar = tvar;
	e->l = 0;
	e->r = 0;
	e->co.cond = 0;
	e->tuple = 0;
	return e;
}

#define lc_in_range(l, r) ((e->code >= LE_##l && e->code <= LE_##r))

struct LocalExpr *copy_local_expr(struct LocalExpr *e) {
	uint32_t i;
	struct LocalExpr *copy = new_local_expr(e->code, e->type, e->tvar);

	if (lc_in_range(BIN_MUL, AFTER_PIPE_LINE)) {
	copy_l_and_r:
		copy->l = copy_local_expr(e->l);
		copy->r = copy_local_expr(e->r);
		if (lce(BIN_TERRY))
			copy->co.cond = copy_local_expr(e->co.cond);

	} else if (lce(AFTER_CALL)) {
		copy->l = copy_local_expr(e->l);
	copy_ops:
		for (i = 0; i < e->co.ops->size; i++)
			plist_add(copy->co.ops, copy_local_expr(plist_get(e->co.ops, i)));

	} else if (lce(AFTER_FIELD_OF_PTR) || lce(AFTER_FIELD)) {
		copy->l = copy_local_expr(e->l);
		copy->r = e->r;

	} else if (lce(AFTER_INDEX)) {
		goto copy_l_and_r;
	} else if (lce(AFTER_INC) || lce(AFTER_DEC) || lce(AFTER_ENUM)) {
		copy->l = copy_local_expr(e->l);
	} else if (lce(PRIMARY_ARR)) {
		goto copy_ops;
	} else if (is_unary(e) || lce(BOOL)) {
		copy->l = copy_local_expr(e->l);
	}

	return copy;
}

struct LEtoT {
	enum LE_Code le;
	enum TCode t;
};
#define leto(t)                                                                \
	{ LE_BIN_##t, t }
#define leto_ass(t)                                                            \
	{ LE_BIN_##t, t##E }

const struct LEtoT lets[] = {
	leto(MUL),
	leto(DIV),
	leto(MOD),
	leto(WHOLE_DIV),
	{LE_BIN_ADD, PLUS},
	{LE_BIN_SUB, MINUS},
	leto(SHL),
	leto(SHR),
	leto(LESS),
	leto(LESSE),
	leto(MORE),
	leto(MOREE),
	{LE_BIN_EQUALS, EQUE},
	{LE_BIN_NOT_EQUALS, NEQU},
	{LE_BIN_BIT_AND, AMPER},
	leto(BIT_XOR),
	leto(BIT_OR),
	leto(AND),
	leto(OR),
	//{LE_BIN_TERRY, QUEST},
	{LE_BIN_ASSIGN, EQU},
	{LE_AFTER_PIPE_LINE, PIPE_LINE},
	{LE_BIN_ADD, PLUSE},
	{LE_BIN_SUB, MINUSE},
	leto_ass(MUL),
	leto_ass(DIV),
	leto_ass(SHL),
	leto_ass(SHR),
	leto_ass(BIT_AND),
	leto_ass(BIT_OR),
	leto_ass(BIT_XOR),
	leto_ass(MOD),
	leto_ass(AND),
	leto_ass(OR),
	{LE_BIN_EQUALS, EQUEE},
	{LE_BIN_NOT_EQUALS, NEQUE},
};

int find_let(struct LocalExpr *e, enum TCode op_code) {
	const struct LEtoT *let;
	uint32_t i;

	for (i = 0, let = lets; i < loa(lets); i++, let++) {
		if (let->t == op_code) {
			e->code = let->le;
			return 1;
		}
	}
	return 0;
}

struct LocalExpr *local_bin(struct LocalExpr *l, struct LocalExpr *r,
							struct Token *op) {
	struct LocalExpr *e = new_local_expr(LE_NONE, 0, op);
	e->l = l;
	e->r = r;

	if (!find_let(e, op->code))
		eet(op, "че за op", 0);
	return e;
}

#define binop(prev_fun, cond)                                                  \
	do {                                                                       \
		struct LocalExpr *e = prev_fun(p);                                     \
		struct Token *c;                                                       \
                                                                               \
		forever {                                                              \
			c = pser_cur(p);                                                   \
                                                                               \
			if (cond) {                                                        \
				consume(p);                                                    \
				e = local_bin(e, prev_fun(p), c);                              \
			} else                                                             \
				break;                                                         \
		}                                                                      \
                                                                               \
		return e;                                                              \
	} while (0)

// cce - C->Code Equal
#define cce(op) (c->code == (op))
#define ops1(o1) (cce(o1))
#define ops2(o1, o2) (cce(o1) || cce(o2))
#define ops3(o1, o2, o3) (cce(o1) || cce(o2) || cce(o3))
#define ops4(o1, o2, o3, o4) (cce(o1) || cce(o2) || cce(o3) || cce(o4))

// Binop Function
#define bf(name, next, ops)                                                    \
	struct LocalExpr *name(struct Pser *p) { binop(next, ops); }

constr BACK = "назад";
constr STEP1 = "шаг";
constr STEP2 = "шагом";
constr EXPECTED_DDD_OR_DDE = "Ожидалось '...' или '..='.";
constr VERB_IS = "есть";
constr THEN = "тогда";

struct LocalExpr *parse_range(struct Pser *p, struct LocalExpr *assignable_e,
							  struct Token *c) {
	// assignable_e => (e0[...|..=]e1 [шаг|шагом e2] [назад]) ( ... )
	// e->l is e0
	// e->r is e1
	// e->co.cond is e2
	// e->flags is ... or ..= and back or not
	// e->tvar->num is assignable_e
	// e->tvar->real is block
	// e->tvar->str is loop

	struct LocalExpr *e = new_local_expr(LE_RANGE_LOOP, 0, c);
	match(pser_cur(p), PAR_L);
	e->tvar->num = (long)assignable_e;

	e->l = local_expression(p);
	c = pser_cur(p);
	ops1(DDD)	? (e->flags = LOOP_DDD)
	: ops1(DDE) ? (e->flags = LOOP_DDE)
				: (eet(c, EXPECTED_DDD_OR_DDE, 0));
	consume(p);
	e->r = local_expression(p);

	c = pser_cur(p);
	sc(vs(c), STEP1)   ? (consume(p), e->co.cond = local_expression(p))
	: sc(vs(c), STEP2) ? (consume(p), e->co.cond = local_expression(p))
					   : (0);

	c = pser_cur(p);
	if (ops1(ID) && sc(vs(c), BACK))
		consume(p), e->flags |= LOOP_IS_BACKWARD;

	match(pser_cur(p), PAR_R);

	struct Loop *loop = new_loop(0, 0);
	plist_add(p->loops, loop);
	e->tvar->str = (void *)loop;
	e->tvar->real = (double)(long)new_plist(16);
	parse_block_of_local_inst(p, (void *)(long)e->tvar->real);
	p->loops->size--; // remove loop from loops

	return e;
}

struct LocalExpr *after_l_expression(struct Pser *p) {
	struct LocalExpr *e = prime_l_expression(p), *after;
	struct Token *c;
	int i;

repeat_after:
	after = 0;
	c = pser_cur(p);

	if (ops2(FIELD_ARROW, SOBAKA_ARROW)) {
		// e->l is struct
		// e->r is token of field name

		after = new_local_expr(
			ops1(FIELD_ARROW) ? LE_AFTER_FIELD_OF_PTR : LE_AFTER_FIELD, 0, c);
		after->l = e;

		if ((c = absorb(p))->code != ID)
			eet(c, FIELD_NAME_CAN_BE_ONLY_ID, 0);
		after->r = (struct LocalExpr *)c;
		consume(p);
	} else if (ops1(PAR_L)) {
		// e->l is called
		// e->ops is params

		after = new_local_expr(LE_AFTER_CALL, 0, c);
	parse_params:
		after->l = e;
		after->co.ops = new_plist(2);

		for (c = absorb(p); !ops2(PAR_R, EF);) {
			plist_add(after->co.ops, local_expression(p));

			c = pser_cur(p);
			if (ops1(COMMA))
				c = absorb(p);
		}
		if (ops1(EF))
			eet(c, "EOF IN after_l_expression fun call", 0);
		consume(p); // skip ')'
	} else if (ops1(DD)) {
		// same as call

		c = absorb(p); // skip '..'
		expect(c, PAR_L);
		after = new_local_expr(LE_NUMEROUS_CALL, 0, c);
		goto parse_params;

	} else if (ops1(PAR_C_L)) {
		// e->l is indexer
		// e->r is index

		consume(p); // skip '['
		after = new_local_expr(LE_AFTER_INDEX, 0, c);
		after->l = e;
		after->r = local_expression(p);
		match(pser_cur(p), PAR_C_R);
	} else if (ops2(INC, DEC)) {
		// e->l is expression

		consume(p); // skip '++' or '--'
		after = new_local_expr(ops1(INC) ? LE_AFTER_INC : LE_AFTER_DEC, 0, c);
		after->l = e;
	} else if (ops1(DOT)) {
		consume(p); // skip '.'
		// - var_expr.some_expr
		// - some_expr.some_expr
		// + some_expr.expr_call -> call
		// + var_expr.expr_call -> call
		// + var_expr.var_expr -> enum

		after = after_l_expression(p);
		if (after->code == LE_PRIMARY_VAR) {
			// e->l is var

			if (!lce(PRIMARY_VAR))
				eet(c, WRONG_DOT_USAGE, HOW_TO_USE_DOT);
			e->code = LE_AFTER_ENUM;
			e->l = after;

			after = e; // for if (after) statement below
		} else if (after->code == LE_AFTER_CALL) {
			plist_add(after->co.ops, 0);
			// move all args to right on 1
			for (i = after->co.ops->size - 1; i >= 0; i--)
				plist_set(after->co.ops, i + 1, plist_get(after->co.ops, i));
			// set [0] element to e
			plist_set(after->co.ops, 0, e);
		} else
			eet(c, WRONG_DOT_USAGE, HOW_TO_USE_DOT);
	} else if (ops1(ID)) {
		if (sc(vs(c), VERB_IS)) {
			// e->l is var
			// e->type is var type
			if (!(lcep(VAR)))
				eet(e->tvar, EXPECTED_VAR_EXPRESSION_TO_USE_WITH_VERB_IS, 0);

			consume(p); // skip 'есть'
			after = new_local_expr(LE_DECLARE_VAR, 0, c);
			e->type = type_expr(p);
			after->l = e;
		}
	}

	if (after) {
		e = after;
		goto repeat_after;
	}
	return e;
}

bf(mulng_l_expression, unary_l_expression, ops4(MUL, DIV, MOD, WHOLE_DIV));
bf(addng_l_expression, mulng_l_expression, ops2(PLUS, MINUS));
bf(shtng_l_expression, addng_l_expression, ops2(SHL, SHR));
bf(mlsng_l_expression, shtng_l_expression, ops4(LESS, LESSE, MORE, MOREE));
bf(equng_l_expression, mlsng_l_expression, ops2(EQUE, NEQU));
bf(b_and_l_expression, equng_l_expression, ops1(AMPER));
bf(b_xor_l_expression, b_and_l_expression, ops1(BIT_XOR));
bf(b_or__l_expression, b_xor_l_expression, ops1(BIT_OR));
bf(l_and_l_expression, b_or__l_expression, ops1(AND));
bf(l_or__l_expression, l_and_l_expression, ops1(OR));
struct LocalExpr *trnry_l_expression(struct Pser *p) {
	struct LocalExpr * true, *false;
	struct LocalExpr *e = l_or__l_expression(p), *trnry;
	struct Token *c;

	c = pser_cur(p);

	if (c->code == QUEST) {
		consume(p); // skip '?'

		true = local_expression(p);
		match(pser_cur(p), COLO);
		false = local_expression(p);

		trnry = new_local_expr(LE_BIN_TERRY, 0, c);
		trnry->co.cond = e;
		trnry->l = true;
		trnry->r = false;

		e = trnry;
	} else if (ops1(QQ)) {
		consume(p); // skip '??'
		trnry = new_local_expr(LE_IF_ELSE, 0, c);
		trnry->co.cond = e;

		trnry->l = (void *)new_plist(16);
		parse_block_of_local_inst(p, (void *)trnry->l);

		c = pser_cur(p);
		if (c->code == CC) {
			consume(p); // skip '::'
			trnry->r = (void *)new_plist(16);
			parse_block_of_local_inst(p, (void *)trnry->r);

		} else if (c->code == CQ) {
			trnry->code = LE_IF_ELIF;
			consume(p); // skip ':?'
			trnry->r = local_expression(p);
			if (!is_if(trnry->r))
				eet(c, "Ожидалось выражение с '::' или '?\?' или ':?'.", 0);

		} else
			trnry->code = LE_IF;

		e = trnry;
	} else if (ops1(LOOP_ARROW)) {
		// in parse_range

		consume(p); // skip '=>'
		e = parse_range(p, e, c);
	} else if (ops1(ID)) {
		if (sc(vs(c), THEN)) {
			// e->l is cond
			// e->r is body
			// e->tvar->num is loop

			consume(p); // skip 'тогда'
			trnry = new_local_expr(LE_THEN_LOOP, 0, c);
			trnry->l = e;

			struct Loop *loop = new_loop(0, 0);
			plist_add(p->loops, loop);
			trnry->tvar->num = (long)loop;

			trnry->r = (void *)new_plist(16);
			parse_block_of_local_inst(p, (void *)trnry->r);
			p->loops->size--; // remove loop from loops

			e = trnry;
		}
	}

	return e;
}
struct LocalExpr *asnge_l_expression(struct Pser *p) {
	struct LocalExpr *l, *r;
	struct LocalExpr *e = l = trnry_l_expression(p);
	struct Token *c;

	c = pser_cur(p);

	if (ops4(PLUSE, MINUSE, MULE, DIVE) ||
		ops4(SHLE, SHRE, BIT_ANDE, BIT_ORE) ||
		ops4(BIT_XORE, MODE, ANDE, ORE) || ops2(EQUEE, NEQUE)) {
		consume(p);

		r = local_expression(p);
		r = local_bin(copy_local_expr(l), r, c);

		e = new_local_expr(LE_BIN_ASSIGN, 0, c);
		e->l = l;
		e->r = r;
	}

	return e;
}
struct LocalExpr *assng_l_expression(struct Pser *p) {
	struct LocalExpr *e = asnge_l_expression(p);
	struct Token *c = pser_cur(p);
	if (ops1(EQU)) {
		consume(p);
		e = local_bin(e, local_expression(p), c);
	}
	return e;
}
bf(pipel_l_expression, assng_l_expression, ops1(PIPE_LINE));
