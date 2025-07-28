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
	 (t)->code == TC_UINT32 || (t)->code == TC_UINT64 || (t)->code == TC_VOID)
#define is_real_type(t) ((t)->code == TC_DOUBLE || (t)->code == TC_FLOAT)
#define is_str_type(t)                                                         \
	((t)->code == TC_PTR && ((t)->data.ptr_target->code == TC_UINT8 ||         \
							 (t)->data.ptr_target->code == TC_VOID))

const char *const NUM_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с числовым типом выражения.";
const char *const STR_INCOMPATIBLE_TYPE =
	"Тип переменной не совместим с строковым типом выражения.";
const char *const ARR_SIZES_DO_NOW_MATCH =
	"Рамеры типа переменной массива и самого массива не совпадают.";
const char *const INCOMPATIBLE_TYPES =
	"Типы переменной и выражения несовместимы.";

struct GlobExpr *parse_global_expression(struct Pser *p,
										 struct TypeExpr *type) {
	long some_value;
	struct Token *equ = get_pser_token(p, -1);
	struct GlobExpr *e = global_expression(p);
	struct TypeExpr *tmp_type;

	if (e->code == CT_INT) {
		if (is_int_type(type))
			return e;

		if (is_real_type(type) && type->code > TC_UINT16) {
			e->tvar->fpn = e->tvar->number;
			e->code = CT_REAL;
			return e;
		}
		eet(p->f, equ, NUM_INCOMPATIBLE_TYPE, 0);
	}

	if (e->code == CT_REAL) {
		if (is_real_type(type))
			return e;

		if (is_int_type(type)) {
			e->tvar->number = e->tvar->fpn;
			e->code = CT_INT;
			return e;
		}
		eet(p->f, equ, NUM_INCOMPATIBLE_TYPE, 0);
	}

	if (e->code == CT_STR) {
		if (is_str_type(type))
			return e;

		// assume array
		if (type->code != TC_ARR)
			eet(p->f, equ, STR_INCOMPATIBLE_TYPE, 0);

		// assume uint8 array
		tmp_type = plist_get(type->data.arr, 0);
		if (tmp_type->code != TC_UINT8)
			eet(p->f, equ, STR_INCOMPATIBLE_TYPE, 0);

		// asume array size
		some_value = (long)plist_get(type->data.arr, 1);
		if (some_value != -1 && some_value != e->tvar->str->size)
			// TODO: make it warn but need to implement warns before
			eet(p->f, equ, ARR_SIZES_DO_NOW_MATCH, 0);

		// set size in any way
		some_value = e->tvar->view->size - 2;
		plist_set(type->data.arr, 1, (void *)some_value);
		return e;
	}

	eet(p->f, equ, INCOMPATIBLE_TYPES, 0);
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
	struct Defn *enum_value;
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
		enum_value = find_enum_value(p, c->view);
		if (enum_value) {
			e->code = CT_INT;
			copy_token(e->tvar, c);
			e->tvar->number = (long)enum_value->value;
			consume(p);
			break;
		}

		other_var = find_global_var(p, c->view);
		e->code = other_var->value->code;

		if (other_var == 0)
			eet(p->f, c, GLOBAL_VAR_WAS_NOT_FOUND, 0);
		if (other_var->value == 0) // && other_var->type->code == TC_FUN)
			eet(p->f, c, FUN_VAR_DOESNT_HAVE_VALUE, 0);

		copy_token(e->tvar, other_var->value->tvar);
		e->tvar->p = c->p;

		// REMEMBER not to free pos and view, only token itself and
		// maybe str and view if it was of CT_STR
		if (other_var->value->code == CT_STR) {
			e->tvar->view = copy_str(other_var->value->tvar->view);
			e->tvar->str = copy_str(other_var->value->tvar->str);
		} // why would i free it tough, it global so

		consume(p);
		break;
	case STR:
		e->code = CT_STR;
		copy_token(e->tvar, c);
		e->tvar->view = copy_str(c->view);
		e->tvar->str = copy_str(c->str);
		consume(p);
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
