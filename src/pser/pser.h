#include "../tzer/tzer.h"
#include <stdint.h>

#define copy_to_fst_and_clear_snd(fst, snd)                                    \
	do {                                                                       \
		blat_blist((fst), (snd));                                              \
		blist_clear_free((snd));                                               \
	} while (0)

extern const char *const STRUCT_NAME_WASNT_FOUND;
extern uc NEED_WARN;
void pw(struct Token *t, const char *const msg, const char *const sgst);
void etei_with_extra(struct ErrorInfo *info);
#define MAX_ARGS_ON_REGISTERS 7

// IP_DECLARE_STRUCT
#define DCLR_STRUCT_NAME 0
#define DCLR_STRUCT_SIZE 1
#define DCLR_STRUCT_MEMS 2
#define DCLR_STRUCT_ARGS 3

extern const char *const EXPECTED__STR;
extern const char *const EXPECTED__PAR_L;
extern const char *const EXPECTED__PAR_R;
extern const char *const EXPECTED__PAR_C_L;
extern const char *const EXPECTED__PAR_C_R;
extern const char *const EXPECTED__EQU;
extern const char *const EXPECTED__COLO;
extern const char *const EXPECTED__ID;
extern const char *const EXPECTED__INT;
extern const char *const EXPECTED__FPN;
extern const char *const EXPECTED__COMMA;
extern const char *const EXPECTED__SH_L;

extern const char *const SUGGEST__STR;
extern const char *const SUGGEST__PAR_L;
extern const char *const SUGGEST__PAR_R;
extern const char *const SUGGEST__PAR_C_L;
extern const char *const SUGGEST__PAR_C_R;
extern const char *const SUGGEST__EQU;
extern const char *const SUGGEST__COLO;
extern const char *const SUGGEST__ID;
extern const char *const SUGGEST__INT;
extern const char *const SUGGEST__FPN;
extern const char *const SUGGEST__COMMA;
extern const char *const SUGGEST__SH_L;

extern const char *const ERR_WRONG_TOKEN;

extern const char *const STR_LET;
extern const char *const STR_ASM;
extern const char *const STR_GOTO;
extern const char *const STR_LOOP;

extern const char *const STR_AS;
extern const char *const STR_SIZE_OF;
extern const char *const STR_SIZE_OF_VAL;

struct Pser {
	struct Fpfc *f;
	struct PList *ts; // tokens
	size_t pos;
	uc debug;

	struct PList *errors;
	struct PList *warns;

	struct PList *enums; // #define's

	struct PList *global_vars; // global variables
	struct PList *local_vars;
};
extern struct PList *parsed_structs; // Inst's of IP_DECLARE_STRUCT

#define pser_need_err(p) ((p)->errors->size != 0 || (p)->warns->size != 0)
void pser_err(struct Pser *p);

struct Pser *new_pser(struct Fpfc *, struct PList *, char *, uc);
struct PList *pse(struct Pser *); // instructions
struct Token *next_pser_get(struct Pser *, long);
struct Token *get_pser_token(struct Pser *, long);
struct PList *find_lik_os(struct BList *name);
#define pser_by(p, ppos) (get_pser_token((p), (ppos) - (p)->pos))
#define expect(t, c)                                                           \
	do {                                                                       \
		if ((t)->code != (c))                                                  \
			eet((t), EXPECTED__##c, SUGGEST__##c);                             \
	} while (0)

#define consume(p) ((p)->pos++)
#define pser_cur(p) (get_pser_token((p), 0))
#define pser_next(p) (get_pser_token((p), 1))
#define absorb(p) (next_pser_get((p), 0)) // consume + ret pser_cur
#define not_ef_and(cd, c) ((c)->code != (cd) && (c)->code != EF)
#define not_ef_and_and(cd1, cd2, c) ((c)->code != (cd1) && not_ef_and((cd2), (c))
#define match(t, c)                                                            \
	do {                                                                       \
		expect(t, c);                                                          \
		consume(p);                                                            \
	} while (0)

enum IP_Code {
	// directives
	IP_NONE,
	IP_EOI, // end of instructions
	// any level
	IP_ASM,
	// global level
	IP_DECLARE_ENUM,
	IP_DECLARE_STRUCT,
	IP_DECLARE_FUNCTION,
	// local level
	IP_LET,

	IP_DECLARE_LABEL,
	IP_GOTO,

	IP_IF_ELIF_ELSE,
	IP_MATCH,

	IP_LOOP,
	IP_FOR_LOOP,
	IP_WHILE_LOOP,

	IP_EXPRESION,
};

struct Defn {
	struct BList *view;
	void *value;
};

struct Word {
	char *view;
	enum IP_Code inst;
};

struct Inst {
	enum IP_Code code;
	struct Fpfc *f;
	struct Token *start_token;
	struct PList *os;
};

enum TypeCode {
	// 8
	TC_INT8,
	TC_UINT8,
	// 16
	TC_INT16,
	TC_UINT16,
	// 32
	TC_INT32, // also enum
	TC_UINT32,
	TC_ENUM,
	TC_FLOAT,
	// 64
	TC_VOID,
	TC_DOUBLE,
	TC_INT64,
	TC_UINT64,

	TC_PTR, // also str str if TC_UINT8 ptr
	TC_FUN,

	TC_ARR,
	TC_STRUCT,
};

struct TypeWord {
	char *view;
	enum TypeCode code;
	uint32_t view_len;
};

union TypeData {
	struct TypeExpr *ptr_target;
	struct BList *name;
	struct PList *args_types;
	struct PList *arr;
};

// it seems that type expr and type size and should be part of one structure yet
// when creating a new Arg with a type and having only pointer to the Лик this
// Лик can be not yet declared and you dont need its size also, so you need
// size of a type only when you need it, not in type parsing time
struct TypeExpr {
	// type code
	enum TypeCode code;
	/*
	 * if [[ ptr ]] -> TypeExpr *
	 * if [[ struct ]] -> name blist
	 * if [[ fun ]] -> plist of TypeExpr * where last type is return type
	 * if [[ arr ]] -> plist with two items
	 * - first item is TypeExpr *
	 * - second item is long that is len of arr, if len is -1 then any len
	 */
	union TypeData data;
};

#define arr_type(t) (((struct TypeExpr *)plist_get((t)->data.arr, 0)))
#define arr_len(t) (plist_get((t)->data.arr, 1))
#define ptr_targ(t) (((struct TypeExpr *)(t)->data.ptr_target))

#define is_void_ptr(t) ((t)->code == TC_PTR && ptr_targ((t))->code == TC_VOID)
#define is_ptr_type(t) ((t)->code == TC_PTR || (t)->code == TC_FUN)

#define set_arr_len(arr, len) (plist_set((arr), 1, (void *)(len)))

struct TypeExpr *new_type_expr(enum TypeCode);
struct TypeExpr *copy_type_expr(struct TypeExpr *type);
void free_type(struct TypeExpr *type);
int size_of_type(struct Pser *p, struct TypeExpr *type);
int unsafe_size_of_type(struct TypeExpr *type);

struct Arg {
	struct PList *names; // PList of Tokens
	long offset;

	struct TypeExpr *type;
	int arg_size;
};

struct PLocalVar {
	struct Token *name; // name is equ of offset in asm so dont need off in here

	struct TypeExpr *type;
	int var_size;
};

struct PLocalVar *new_plocal_var(struct Token *, struct Arg *);

enum CE_Code {
	CE_NONE,
	CE_NUM_INCOMPATIBLE_TYPE,
	CE_STR_INCOMPATIBLE_TYPE,
	CE_ARR_SIZES_DO_NOW_MATCH,
	CE_PTR_INCOMPATIBLE_TYPE,
	CE_FUN_INCOMPATIBLE_TYPE,
	CE_ARR_INCOMPATIBLE_TYPE,
	CE_STRUCT_INCOMPATIBLE_TYPE,
	CE_AS_INCOMPATIBLE_TYPE,
	CE_ARR_ITEM_INCOMPATIBLE_TYPE,
	CE_ARR_FROM_OTHER_GLOBAL_ARR,
	CE_STRUCT_FROM_OTHER_GLOBAL_STRUCT,
	CE_TOO_MUCH_FIELDS_FOR_THIS_STRUCT,
	CE_TOO_LESS_FIELDS_FOR_THIS_STRUCT,
	CE_TOO_MUCH_ITEMS_FOR_THIS_ARR,
	CE_TOO_LESS_ITEMS_FOR_THIS_ARR,
	CE_TOO_MUCH_CHARS_FOR_THIS_STR,
	CE_TOO_LESS_CHARS_FOR_THIS_STR,
	CE_CANT_DEFINE_ARR_TYPE,
	CE_CANT_DEFINE_STRUCT_TYPE,
	CE_STRUCT_WASNT_FOUND,
	CE_STR_IS_NOT_A_PTR,
	CE_ARR_IS_NOT_A_PTR,
	CE_UNCOMPUTIBLE_DATA,
	CE_EXCESSING_FIELD,

	CE_todo,
};

// Compilation Time
enum CT_Code {
	CT_INT,
	CT_REAL,
	CT_FUN,

	CT_STR,
	CT_STR_PTR,
	CT_ARR,
	CT_ARR_PTR,
	CT_STRUCT,
	CT_STRUCT_PTR,
	CT_GLOBAL, // invalid cuz uncomputable !yet! but needed to get CT_GLOBAL_PTR
	CT_GLOBAL_PTR, // pointer to other global value, is it exist?

	// to fill lost values in array or struct, haves only code and type
	// its need a type to gen zero bytes by type size
	CT_ZERO,
};
#define is_compile_time_ptr(e)                                                 \
	((e)->code == CT_STR_PTR || (e)->code == CT_ARR_PTR ||                     \
	 (e)->code == CT_STRUCT_PTR || (e)->code == CT_GLOBAL_PTR)

struct GlobExpr {
	struct GlobVar *from;
	uc not_from_child;
	uc struct_with_fields;

	enum CT_Code code;
	struct TypeExpr *type; // or 0
	struct Token *tvar;	   // тварь
	// list of GlobExpr's or 0 or NamedStructField's if struct_with_fields
	struct PList *globs;
};
void free_glob_expr(struct GlobExpr *e);

struct NamedStructField {
	struct Token *name_token;
	struct GlobExpr *expression;
};
void global_single_struct(struct Pser *p, struct GlobExpr *e, struct Token *c);

#define TOO_MUCH_ITEMS 1
#define NEED_ADD_ITEMS -1
void are_types_compatible(struct PList *, struct TypeExpr *, struct GlobExpr *);
int arr_err_of_size(struct PList *, struct GlobExpr *, long size, long items);
void cmpt_int(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_real(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_fun(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_str(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_str_ptr(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_arr(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_arr_ptr(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_global(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_global_ptr(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_struct(struct PList *, struct TypeExpr *, struct GlobExpr *);
void cmpt_struct_ptr(struct PList *, struct TypeExpr *, struct GlobExpr *);
// void cmpt_zero(struct PList *, struct TypeExpr *, struct GlobExpr *);

struct GlobVar {
	struct Token *name;
	struct BList *signature;
	struct GlobExpr *value;
	// it cant be equal to name cuz its label not to value of the variabel but
	// to the value of the value of variable and if value of variable is just a
	// value not ptr then its important, also it cant be used as value in local
	// expression cuz ots not const value and acan be changeable
	struct BList *value_label;

	struct TypeExpr *type;
	int gvar_size;
};

#define types_sizes_do_match(t1, t2)                                           \
	(((t1) >= TC_VOID && (t2) >= TC_VOID) ||                                   \
	 ((t1) >= TC_INT32 && (t2) >= TC_INT32 && (t1) < TC_VOID &&                \
	  (t2) < TC_VOID) ||                                                       \
	 ((t1) >= TC_INT16 && (t2) >= TC_INT16 && (t1) < TC_INT32 &&               \
	  (t2) < TC_INT32) ||                                                      \
	 ((t1) >= TC_INT8 && (t2) >= TC_INT8 && (t1) < TC_INT16 &&                 \
	  (t2) < TC_INT16))

#define copy_token(d, s) (memcpy((d), (s), sizeof(struct Token)))
struct Arg *get_arg_by_mem_index(struct PList *lik_os, uint32_t mem_index);
struct Arg *get_arg_of_next_offset(struct PList *lik_os, long last_offset);
struct PList *copy_globs(struct PList *globs);
void search_error_code(struct Pser *p, struct PList *msgs);
void *expression(struct Pser *);
struct GlobExpr *parse_global_expression(struct Pser *p, struct TypeExpr *type);
void parse_args(struct Pser *p, struct PList *os);
struct TypeExpr *type_expr(struct Pser *);
struct Inst *new_inst(struct Pser *, enum IP_Code, struct PList *os,
					  struct Token *);

struct Inst *get_global_inst(struct Pser *p);
// enum IP_Code inst_pser_define(struct Pser *p, struct PList *os);
// enum IP_Code inst_pser_include(struct Pser *p, struct PList *os);
enum IP_Code inst_pser_asm(struct Pser *p, struct PList *os);
enum IP_Code inst_pser_enum(struct Pser *p, struct PList *os);
enum IP_Code inst_pser_struct(struct Pser *p, struct PList *os);
enum IP_Code inst_pser_dare_fun(struct Pser *p, struct PList *os);
enum IP_Code inst_pser_global_let(struct Pser *p, struct PList *os);

void parse_block_of_local_inst(struct Pser *p, struct PList *os);
struct Inst *get_local_inst(struct Pser *p);

void get_fun_signature_considering_args(struct PList *os, struct GlobVar *var);
void get_global_signature(struct GlobVar *var);
int are_types_equal(struct TypeExpr *, struct TypeExpr *);
void check_list_of_args_on_uniq_names(struct PList *l, uint32_t start_index);
void check_list_of_args_on_name(struct PList *l, uint32_t from_arg,
								uint32_t from_name,
								struct Token *name_to_check);
void check_list_of_vars_on_name(struct Pser *p, struct Token *name_to_check);
long unsafe_size_of_global_value(struct GlobExpr *e);

void eei(struct Inst *, const char *const msg, const char *const sgst);

#define is_int_type(t)                                                         \
	((t)->code == TC_INT8 || (t)->code == TC_INT16 || (t)->code == TC_INT32 || \
	 (t)->code == TC_INT64 || (t)->code == TC_VOID || (t)->code == TC_ENUM ||  \
	 (t)->code == TC_UINT8 || (t)->code == TC_UINT16 ||                        \
	 (t)->code == TC_UINT32 || (t)->code == TC_UINT64)
#define is_real_type(t) ((t)->code == TC_DOUBLE || (t)->code == TC_FLOAT)

void check_global_type_compatibility(struct Pser *p, struct TypeExpr *type,
									 struct GlobExpr *e);

struct GlobExpr *global_bin(struct Pser *p, struct GlobExpr *l,
							struct GlobExpr *r, struct Token *op);

struct GlobExpr *after_g_expression(struct Pser *p);
struct GlobExpr *prime_g_expression(struct Pser *p);
struct GlobExpr *unary_g_expression(struct Pser *p);
struct GlobExpr *mulng_g_expression(struct Pser *p);
struct GlobExpr *addng_g_expression(struct Pser *p);
struct GlobExpr *shtng_g_expression(struct Pser *p);
struct GlobExpr *mlsng_g_expression(struct Pser *p);
struct GlobExpr *equng_g_expression(struct Pser *p);
struct GlobExpr *b_and_g_expression(struct Pser *p);
struct GlobExpr *b_xor_g_expression(struct Pser *p);
struct GlobExpr *b_or__g_expression(struct Pser *p);
struct GlobExpr *l_and_g_expression(struct Pser *p);
struct GlobExpr *l_or__g_expression(struct Pser *p);
struct GlobExpr *trnry_g_expression(struct Pser *p);
#define global_expression(p) (trnry_g_expression((p)))
