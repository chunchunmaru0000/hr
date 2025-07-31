#include "pser.h"

long a = (long)(&"str");

struct GlobExpr *after_g_expression(struct Pser *p);
struct GlobExpr *prime_g_expression(struct Pser *p);
struct GlobExpr *unary_g_expression(struct Pser *p);
// struct GlobExpr *mulng_g_expression(struct Pser *p);
struct GlobExpr *addng_g_expression(struct Pser *p);
// struct GlobExpr *booln_g_expression(struct Pser *p);
#define global_expression(p) (addng_g_expression((p)))

const char *const NUM_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с числовым типом выражения.";
const char *const STR_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с строковым типом выражения.";
const char *const ARR_SIZES_DO_NOW_MATCH =
	"Рамеры типа переменной массива и самого массива не совпадают.";
const char *const PTR_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с указываемым типом выражения.";
const char *const FUN_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с функциональным типом выражения.";
const char *const INCOMPATIBLE_TYPES =
	"Типы переменной и выражения несовместимы.";
const char *const UNCOMPUTIBLE_DATA = "Невычислимое выражение.";

struct GlobExpr *parse_global_expression(struct Pser *p,
										 struct TypeExpr *type) {
	struct Token *equ = get_pser_token(p, -1);
	struct GlobExpr *e = global_expression(p);

	enum CE_Code error = are_types_compatible(type, e);

	switch (error) {
	case CE_NONE:
		break;
	case CE_NUM_INCOMPATIBLE_TYPE:
		eet(p->f, equ, NUM_INCOMPATIBLE_TYPE, 0);
	case CE_STR_INCOMPATIBLE_TYPE:
		eet(p->f, equ, STR_INCOMPATIBLE_TYPE, 0);
	case CE_ARR_SIZES_DO_NOW_MATCH:
		pw(p->f, equ->p, ARR_SIZES_DO_NOW_MATCH);
		break;
	case CE_PTR_INCOMPATIBLE_TYPE:
		eet(p->f, equ, PTR_INCOMPATIBLE_TYPE, 0);
	case CE_FUN_INCOMPATIBLE_TYPE:
		eet(p->f, equ, FUN_INCOMPATIBLE_TYPE, 0);
	case CE_UNCOMPUTIBLE_DATA:
		eet(p->f, equ, UNCOMPUTIBLE_DATA, 0);

	case CE_TODO1:
		eet(p->f, equ, "TODO1", 0);
	case CE_TODO2:
		eet(p->f, equ, "TODO2", 0);
	case CE_TODO3:
		eet(p->f, equ, "TODO3", 0);
	case CE_TODO4:
		eet(p->f, equ, "TODO4", 0);
	}

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

#define copy_token(d, s) (memcpy((d), (s), sizeof(struct Token)))

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
			if (other_var->type->code == TC_FUN)
				e->code = CT_FUN;
			else
				e->code = CT_GLOBAL;
			copy_token(e->tvar, c);
			break;
		}

		e->code = other_var->value->code;
		copy_token(e->tvar, other_var->value->tvar);
		e->tvar->p = c->p;

		// REMEMBER: not to free pos and view, only token itself and
		// maybe view if it was of CT_STR
		if (other_var->value->code == CT_STR)
			e->tvar->view = copy_str(other_var->value->tvar->view);
		// why would i free it tough, it global so

		break;
	case STR:
		e->code = CT_STR;
		copy_token(e->tvar, c);
		e->tvar->view = copy_str(c->view);
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

		if (!e->from)
			eet(p->f, c, CANT_TAKE_PTR_FROM_NOT_GVAR, 0);

		e->code = CT_GLOBAL_PTR;

		return e;
	}

	if (c->code == ID && sc(STR_AS, (char *)c->view->st)) {
		consume(p); // skip окак

		type = type_expr(p);
		e = unary_g_expression(p);

		if (e->type) {
			// TODO: if types are conflict
		}
		e->type = type;

		return e;
	}

	return after_g_expression(p);
}

struct GlobExpr *addng_g_expression(struct Pser *p) {
	struct GlobExpr *e = unary_g_expression(p);

	return e;
}
