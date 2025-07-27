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

struct GlobExpr *parse_global_expression(struct Pser *p,
										 struct TypeExpr *type) {
	struct GlobExpr *e = global_expression(p);
	// enum Comp compatibility = get_types_compatibility(e->type, arg->type);

	if (e->code == CT_INT) {
		if (is_int_type(type))
			return e;
		if (is_real_type(type)) {
			e->tvar->fpn = e->tvar->number;
		}
		eet(p->f, pser_cur(p), "1. TODO parse_global_expression", "TODO");
	}
	if (e->code == CT_REAL) {
		if (is_real_type(type))
			return e;
		if (is_int_type(type)) {
			e->tvar->number = e->tvar->fpn;
		}
		eet(p->f, pser_cur(p), "2. TODO parse_global_expression", "TODO");
	}

	eet(p->f, pser_cur(p), "3. TODO parse_global_expression", "TODO");
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

struct GlobExpr *after_g_expression(struct Pser *p) {
	struct GlobExpr *e = addng_g_expression(p);

	return e;
}

#define copy_token(d, s) (memcpy((d), (s), sizeof(struct Token)))

const char *const GLOBAL_VAR_WAS_NOT_FOUND =
	"Глобальная переменная с таким именем не была найдена.";
const char *const FUN_VAR_DOESNT_HAVE_VALUE =
	"Функция как переменная не может иметь значение.";

struct GlobExpr *prime_g_expression(struct Pser *p) {
	struct GlobVar *other_var;
	struct BList *str;
	struct Token *c = pser_cur(p);

	struct GlobExpr *e = malloc(sizeof(struct GlobExpr));
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
		other_var = find_global_var(p, c->view);
		e->code = other_var->value->code;

		if (other_var == 0)
			eet(p->f, c, GLOBAL_VAR_WAS_NOT_FOUND, 0);
		if (other_var->value == 0) // && other_var->type->code == TC_FUN)
			eet(p->f, c, FUN_VAR_DOESNT_HAVE_VALUE, 0);

		copy_token(e->tvar, other_var->value->tvar);
		e->tvar->p = c->p;
		// REMEMBER not to free pos and view, only token itself and
		// maybe str if it was of CT_STR

		if (other_var->value->code == CT_STR) {
			str = new_blist(32);
			blat_blist(str, other_var->value->tvar->str);
			convert_blist_to_blist_from_str(str);
			e->tvar->str = str;
		}

		consume(p);
		break;
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
			eet(p->f, c, "1. TODO unary_expression", "TODO");

		return e;
	}
	if (c->code == MINUS) {
		consume(p);
		e = unary_g_expression(p);

		if (e->code == CT_INT) {
			e->tvar->number *= -1;
		} else if (e->code == CT_REAL) {
			e->tvar->fpn *= -1;
		} else {
			eet(p->f, c, "3. TODO unary_expression", "TODO");
		}
		return e;
	}

	return prime_g_expression(p);
}

struct GlobExpr *addng_g_expression(struct Pser *p) {
	struct GlobExpr *e = unary_g_expression(p);

	return e;
}
