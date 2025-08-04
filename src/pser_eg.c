#include "pser.h"

// TODO: long a = (long)("str");

struct GlobExpr *after_g_expression(struct Pser *p);
struct GlobExpr *prime_g_expression(struct Pser *p);
struct GlobExpr *unary_g_expression(struct Pser *p);
// struct GlobExpr *mulng_g_expression(struct Pser *p);
struct GlobExpr *addng_g_expression(struct Pser *p);
// struct GlobExpr *booln_g_expression(struct Pser *p);
#define global_expression(p) (addng_g_expression((p)))

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
const char *const CANT_TAKE_PTR_FROM_NOT_GVAR =
	"Нельзя получить адрес не глобальной переменной";

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
			e->tvar->number = (long)enum_value->value;
			break;
		}

		other_var = find_global_var(p, c->view);
		if (other_var == 0)
			eet(p->f, c, GLOBAL_VAR_WAS_NOT_FOUND, (char *)c->view->st);

		e->from = other_var;

		if (other_var->value == 0) {
		make_primary_id_global:

			if (other_var->type->code == TC_FUN)
				e->code = CT_FUN;
			else
				e->code = CT_GLOBAL;
			copy_token(e->tvar, c);
			break;
		}
		// if (other_var->value->code == CT_ARR)
		// 	goto make_primary_id_global;

		e->code = other_var->value->code;
		copy_token(e->tvar, other_var->value->tvar);
		e->tvar->p = c->p;

		// REMEMBER: not to free pos and view, only token itself and
		// maybe view if it was of CT_STR
		if (other_var->value->code == CT_STR ||
			other_var->value->code == CT_STR_PTR) {
			// why would i free it tough, it global so
			e->tvar->view = copy_str(other_var->value->tvar->view);
			e->tvar->str = copy_str(other_var->value->tvar->str);
		} // else if (other_var->value->code == CT_ARR) {
		  //  need to copy them cuz values can change while checking types
		  //  compatibility, like real and int values
		  // e->globs = copy_globs(other_var->value->globs);
		//} its error case when compare types

		break;
	case STR:
		e->code = CT_STR;
		copy_token(e->tvar, c);
		e->tvar->view = copy_str(c->view);
		e->tvar->str = copy_str(c->str);
		consume(p);
		break;
	case PAR_T_L:
		e->code = CT_STRUCT;
		e->tvar = c; // HERE TOKEN IS NOT COPIED
		e->globs = new_plist(2);

		consume(p);
		while (not_ef_and(PAR_T_R, pser_cur(p)))
			plist_add(e->globs, global_expression(p));
		consume(p);

		break;
	case PAR_C_L:
		e->code = CT_ARR;
		e->tvar = c; // HERE TOKEN IS NOT COPIED
		e->globs = new_plist(2);

		consume(p);
		while (not_ef_and(PAR_C_R, pser_cur(p)))
			plist_add(e->globs, global_expression(p));
		consume(p);

		break;
	case PAR_L:
		consume(p);
		free(e);
		e = global_expression(p);
		match(p, pser_cur(p), PAR_R);
		break;
	default:
		eet(p->f, c, UNEXPECTED_TOKEN_IN_GLOB_EXPR, 0);
	}

	return e;
}

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
			e->tvar->number *= -1;
		} else if (e->code == CT_REAL) {
			e->tvar->fpn *= -1;
		} else {
			eet(p->f, c, NOT_NUM_VALUE_FOR_THIS_UNARY_OP, 0);
		}
		return e;
	}

	if (c->code == AMPER) {
		consume(p);
		e = after_g_expression(p);

		if (e->code == CT_STR)
			e->code = CT_STR_PTR;
		else if (e->code == CT_ARR)
			// TODO: fix in herer or not here i dunno for now, need to commit and go
			e->code = e->from ? CT_GLOBAL_PTR : CT_ARR_PTR;
		else if (e->from)
			e->code = CT_GLOBAL_PTR;
		else
			eet(p->f, c, CANT_TAKE_PTR_FROM_NOT_GVAR, 0);

		return e;
	}

	if (c->code == ID && sc(STR_AS, (char *)c->view->st)) {
		consume(p); // skip окак

		type = type_expr(p);
		e = unary_g_expression(p);

		check_global_type_compatibility(p, type, e);

		if (e->type)
			free_type(e->type);
		e->type = type;

		return e;
	}

	return after_g_expression(p);
}

struct GlobExpr *addng_g_expression(struct Pser *p) {
	struct GlobExpr *e = unary_g_expression(p);
	// TODO: maybe take strings in globs and not add them in one ?
	return e;
}
