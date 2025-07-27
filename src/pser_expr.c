#include "pser.h"

long a = (long)(&"str");

struct GlobExpr *after_g_expression(struct Pser *p);
struct GlobExpr *prime_g_expression(struct Pser *p);
struct GlobExpr *unary_g_expression(struct Pser *p);
// struct GlobExpr *mulng_g_expression(struct Pser *p);
struct GlobExpr *addng_g_expression(struct Pser *p);
// struct GlobExpr *booly_g_expression(struct Pser *p);
#define global_expression(p) (after_g_expression((p)))

#define is_int_type(t)                                                         \
	((t)->code == TC_INT8 || (t)->code == TC_INT16 || (t)->code == TC_INT32 || \
	 (t)->code == TC_INT64 || (t)->code == TC_VOID || (t)->code == TC_ENUM ||  \
	 (t)->code == TC_UINT8 || (t)->code == TC_UINT16 ||                        \
	 (t)->code == TC_UINT32 || (t)->code == TC_UINT64)
#define is_real_type(t) ((t)->code == TC_DOUBLE || (t)->code == TC_FLOAT)
// "str" "str" "str" "str"
// num + num - num * num / num
// {struct values}
// окак [ч32] 123
struct GlobExpr *parse_global_expression(struct Pser *p,
										 struct TypeExpr *type) {
	struct GlobExpr *e = global_expression(p);
	struct Token *value;
	// enum Comp compatibility = get_types_compatibility(e->type, arg->type);

	if (e->code == CT_INT) {
		if (is_int_type(type))
			return e;
		if (is_real_type(type)) {
			value = plist_get(e->ops, 0);
			value->fpn = value->number;
		}
		eet(p->f, pser_cur(p), "1. TODO parse_global_expression", "TODO");
	}
	if (e->code == CT_REAL) {
		if (is_real_type(type))
			return e;
		if (is_int_type(type)) {
			value = plist_get(e->ops, 0);
			value->number = value->fpn;
		}
		eet(p->f, pser_cur(p), "2. TODO parse_global_expression", "TODO");
	}

	eet(p->f, pser_cur(p), "3. TODO parse_global_expression", "TODO");
	return e;
}

void *expression(struct Pser *p) { return p; }

struct GlobExpr *after_g_expression(struct Pser *p) {
	struct GlobExpr *e = addng_g_expression(p);

	return e;
}

struct GlobExpr *prime_g_expression(struct Pser *p) {
	struct Token *c = pser_cur(p);

	struct GlobExpr *e = malloc(sizeof(struct GlobExpr));
	e->type = new_type_expr(TC_VOID);
	e->ops = new_plist(2);

	switch (c->code) {
	case INT:
		e->code = CT_INT;
		plist_add(e->ops, c);
		consume(p);
		break;
	case REAL:
		e->code = CT_REAL;
		plist_add(e->ops, c);
		consume(p);
		break;
	case ID:
	case STR:
	case PAR_L:

	default:;
	}

	return e;
}

struct GlobExpr *unary_g_expression(struct Pser *p) {
	struct GlobExpr *e;
	struct Token *c = pser_cur(p);

	if (c->code == PLUS) {
		consume(p);
		e = unary_g_expression(p);

		if (e->code == CT_INT || e->code == CT_REAL)
			;
		else
			eet(p->f, c, "TODO unary_expression", "TODO");

		return e;
	}
	if (c->code == MINUS) {
		consume(p);
		e = unary_g_expression(p);

		if (e->code == CT_INT) {
			c = plist_get(e->ops, 0);
			c->number *= -1;
		} else if (e->code == CT_REAL) {
			c = plist_get(e->ops, 0);
			c->fpn *= -1;
		} else {
			eet(p->f, c, "TODO unary_expression", "TODO");
		}
		return e;
	}

	return prime_g_expression(p);
}

struct GlobExpr *addng_g_expression(struct Pser *p) {
	struct GlobExpr *e = unary_g_expression(p);

	return e;
}
