#include "pser.h"

// TODO: long a = (long)("str");

struct GlobExpr *parse_global_expression(struct Pser *p,
										 struct TypeExpr *type) {
	struct GlobExpr *e = global_expression(p);

	check_global_type_compatibility(p, type, e);
	if (!e->type)
		e->type = type;

	return e;
}

struct GlobVar *find_global_var(struct Pser *p, struct BList *name) {
	struct GlobVar *var;
	uint32_t i;

	for (i = 0; i < p->global_vars->size; i++) {
		var = plist_get(p->global_vars, i);

		if (sc((char *)var->name->view->st, (char *)name->st))
			return var;
	}
	return 0;
}
struct Defn *find_enum_value(struct Pser *p, struct BList *name) {
	struct Defn *enum_value;
	uint32_t i;

	for (i = 0; i < p->enums->size; i++) {
		enum_value = plist_get(p->enums, i);

		if (sc((char *)enum_value->view->st, (char *)name->st))
			return enum_value;
	}
	return 0;
}

struct GlobExpr *after_g_expression(struct Pser *p) {
	struct GlobExpr *e = prime_g_expression(p);

	return e;
}

const char *const UNEXPECTED_TOKEN_IN_GLOB_EXPR =
	"Непредвиденное слово для глобального выражения.";
const char *const GLOBAL_VAR_WAS_NOT_FOUND =
	"Глобальная переменная с таким именем не была найдена.";
const char *const FUN_VAR_DOESNT_HAVE_VALUE =
	"Функция как переменная не может иметь значение.";
const char *const NOT_NUM_VALUE_FOR_THIS_UNARY_OP =
	"Для данной математической операции ожидалось строковое значение.";
const char *const CANT_TAKE_PTR_FROM_THIS =
	"В глобальном выражении адрес можно получить только из: глобальной "
	"переменной или массива.";
const char *const CANT_DEREFERENCE_THIS =
	"Разыменовывать значениние можно только если: это строка для получения "
	"массива или это указатель на лик для получения значения лика.";
const char *const BIT_NOT_WORKS_ONLY_WITH_INT =
	"Побитовое не работает только с целыми числами.";

struct GlobExpr *prime_g_expression(struct Pser *p) {
	struct GlobVar *other_var;
	struct Defn *enum_value;
	struct Token *c = pser_cur(p);

	struct GlobExpr *e = malloc(sizeof(struct GlobExpr));
	e->type = 0;
	e->globs = 0;
	e->from = 0;
	e->tvar = malloc(sizeof(struct Token));

	switch (c->code) {
	case INT:
		e->code = CT_INT;
		copy_token(e->tvar, c);
		consume(p);
		break;
	case REAL:
		e->code = CT_REAL;
		copy_token(e->tvar, c);
		consume(p);
		break;
	case ID:
		consume(p);

		enum_value = find_enum_value(p, c->view);
		if (enum_value) {
			e->code = CT_INT;
			copy_token(e->tvar, c);
			e->tvar->num = (long)enum_value->value;
			break;
		}

		other_var = find_global_var(p, c->view);
		if (other_var == 0)
			eet(p->f, c, GLOBAL_VAR_WAS_NOT_FOUND, (char *)c->view->st);

		e->from = other_var;

		if (other_var->value == 0) {
			if (other_var->type->code == TC_FUN)
				e->code = CT_FUN;
			else {
			e_be_global_and_copy_token_and_break:
				e->code = CT_GLOBAL;
			}
			copy_token(e->tvar, c);
			break;
		}
		if (other_var->value->code == CT_ARR ||
			other_var->value->code == CT_STRUCT)
			goto e_be_global_and_copy_token_and_break;

		e->code = other_var->value->code;
		copy_token(e->tvar, other_var->value->tvar);
		e->tvar->p = c->p;

		// REMEMBER: not to free pos and view, only token itself and
		// maybe view if it was of CT_STR
		if (other_var->value->code == CT_STR ||
			other_var->value->code == CT_STR_PTR) {

			e->tvar->view = copy_str(other_var->value->tvar->view);
			e->tvar->str = copy_str(other_var->value->tvar->str);
		} // else if (other_var->value->code == CT_ARR) {
		  //  need to copy them cuz values can change while checking types
		  //  compatibility, like real and int values
		  // e->globs = copy_globs(other_var->value->globs);
		//} its error case when compare types

		break;
	case STR:
		e->code = CT_STR_PTR;
		copy_token(e->tvar, c);
		e->tvar->view = copy_str(c->view);
		e->tvar->str = copy_str(c->str);
		consume(p);
		break;
	case PAR_T_L:
		e->code = CT_STRUCT_PTR;
		free(e->tvar); // it was malloced above
		e->tvar = c;   // HERE TOKEN IS NOT COPIED
		e->globs = new_plist(2);

		for (c = absorb(p); not_ef_and(PAR_T_R, c);) {
			plist_add(e->globs, global_expression(p));

			c = pser_cur(p);
			if (c->code == COMMA) // delimeter
				c = absorb(p);
		}
		consume(p); // consume PAR_T_R

		break;
	case PAR_C_L:
		e->code = CT_ARR;
		free(e->tvar); // it was malloced above
		e->tvar = c;   // HERE TOKEN IS NOT COPIED
		e->globs = new_plist(2);

		for (c = absorb(p); not_ef_and(PAR_C_R, c);) {
			plist_add(e->globs, global_expression(p));

			c = pser_cur(p);
			if (c->code == COMMA) // delimeter
				c = absorb(p);
		}
		consume(p); // consume PAR_C_R

		break;
	case PAR_L:
		consume(p);
		free(e->tvar); // it was malloced above
		free(e);	   // it was malloced above
		e = global_expression(p);
		match(p, pser_cur(p), PAR_R);
		break;
	default:
		eet(p->f, c, UNEXPECTED_TOKEN_IN_GLOB_EXPR, 0);
	}

	return e;
}

// TODO: sizeof is unary thing
struct GlobExpr *unary_g_expression(struct Pser *p) {
	struct GlobExpr *e;
	struct TypeExpr *type;
	struct Token *c = pser_cur(p);

	if (c->code == PLUS) {
		consume(p);
		e = unary_g_expression(p);

		if (e->code == CT_INT || e->code == CT_REAL)
			;
		else
			eet(p->f, c, NOT_NUM_VALUE_FOR_THIS_UNARY_OP, 0);

		return e;
	}
	if (c->code == MINUS) {
		consume(p);
		e = unary_g_expression(p);

		if (e->code == CT_INT) {
			e->tvar->num *= -1;
		} else if (e->code == CT_REAL) {
			e->tvar->real *= -1;
		} else {
			eet(p->f, c, NOT_NUM_VALUE_FOR_THIS_UNARY_OP, 0);
		}
		return e;
	}
	if (c->code == EXCL) {
		consume(p);
		e = unary_g_expression(p);

		if (e->code == CT_INT)
			e->tvar->num = !e->tvar->num;
		else if (e->code == CT_REAL) {
			e->tvar->num = !e->tvar->real;
			e->code = CT_INT;
		} else
			eet(p->f, c, NOT_NUM_VALUE_FOR_THIS_UNARY_OP, 0);
		return e;
	}
	if (c->code == BIT_NOT) {
		consume(p);
		e = unary_g_expression(p);

		if (e->code != CT_INT)
			eet(p->f, c, BIT_NOT_WORKS_ONLY_WITH_INT, 0);

		e->tvar->num = ~e->tvar->num;

		return e;
	}
	if (c->code == MUL) {
		consume(p);
		e = after_g_expression(p);

		if (e->code == CT_STR_PTR)
			e->code = CT_STR;
		else if (e->code == CT_STRUCT_PTR)
			e->code = CT_STRUCT;
		else if (e->code == CT_ARR_PTR)
			e->code = CT_ARR;
		else if (e->code == CT_GLOBAL_PTR)
			e->code = CT_GLOBAL;
		else
			eet(p->f, c, CANT_DEREFERENCE_THIS, 0);

		return e;
	}
	if (c->code == AMPER) {
		consume(p);
		e = after_g_expression(p);

		if (e->code == CT_STR)
			e->code = CT_STR_PTR;
		else if (e->code == CT_STRUCT)
			e->code = CT_STRUCT_PTR;
		else if (e->code == CT_ARR)
			e->code = CT_ARR_PTR;
		else if (e->from)
			e->code = CT_GLOBAL_PTR;
		else
			eet(p->f, c, CANT_TAKE_PTR_FROM_THIS, 0);

		return e;
	}
	if (c->code == ID) {
		if (sc(STR_AS, (char *)c->view->st)) {
			consume(p); // skip окак

			type = type_expr(p);
			e = unary_g_expression(p);

			check_global_type_compatibility(p, type, e);

			if (e->type)
				free_type(e->type);
			e->type = type;

			return e;
		} else if (sc(STR_SIZE_OF, (char *)c->view->st)) {
			consume(p); // skip мера

			type = type_expr(p);
			long size = unsafe_size_of_type(type);
			free_type(type);

			e = malloc(sizeof(struct GlobExpr));
			e->type = 0;
			e->globs = 0;
			e->from = 0;
			e->tvar = malloc(sizeof(struct Token));
			copy_token(e->tvar, c);
			e->tvar->num = size;
			e->code = CT_INT;

			return e;
		}
	}

	return after_g_expression(p);
}

#define binop(prev_fun, cond)                                                  \
	do {                                                                       \
		struct GlobExpr *e = prev_fun(p);                                      \
		struct Token *c;                                                       \
                                                                               \
		loop {                                                                 \
			c = pser_cur(p);                                                   \
                                                                               \
			if (cond) {                                                        \
				consume(p);                                                    \
				e = global_bin(p, e, prev_fun(p), c);                          \
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
	struct GlobExpr *name(struct Pser *p) { binop(next, ops); }

bf(mulng_g_expression, unary_g_expression, ops3(MUL, DIV, MOD));
bf(addng_g_expression, mulng_g_expression, ops2(PLUS, MINUS));
bf(shtng_g_expression, addng_g_expression, ops2(SHL, SHR));
bf(mlsng_g_expression, shtng_g_expression, ops4(LESS, LESSE, MORE, MOREE));
bf(equng_g_expression, mlsng_g_expression, ops2(EQUE, NEQU));
bf(b_and_g_expression, equng_g_expression, ops1(AMPER));
bf(b_xor_g_expression, b_and_g_expression, ops1(BIT_XOR));
bf(b_or__g_expression, b_xor_g_expression, ops1(BIT_OR));
bf(l_and_g_expression, b_or__g_expression, ops1(AND));
bf(l_or__g_expression, l_and_g_expression, ops1(OR));
