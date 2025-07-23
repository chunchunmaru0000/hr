#include "tzer.h"
#include <stdint.h>

extern uc NEED_WARN;
void pw(struct Fpfc *f, struct Pos *p, const char *const msg);
#define MAX_ARGS_ON_REGISTERS 7

extern const char *const EXPECTED__STR;
extern const char *const EXPECTED__PAR_L;
extern const char *const EXPECTED__PAR_R;
extern const char *const EXPECTED__PAR_C_L;
extern const char *const EXPECTED__PAR_C_R;
extern const char *const EXPECTED__COLO;
extern const char *const EXPECTED__ID;
extern const char *const EXPECTED__INT;
extern const char *const EXPECTED__FPN;

extern const char *const SUGGEST__STR;
extern const char *const SUGGEST__PAR_L;
extern const char *const SUGGEST__PAR_R;
extern const char *const SUGGEST__PAR_C_L;
extern const char *const SUGGEST__PAR_C_R;
extern const char *const SUGGEST__COLO;
extern const char *const SUGGEST__ID;
extern const char *const SUGGEST__INT;
extern const char *const SUGGEST__FPN;

extern const char *const ERR_WRONG_TOKEN;

extern const char *const STR_LET;
extern const char *const STR_ASM;

struct Pser {
	struct Fpfc *f;
	struct PList *ts; // tokens
	size_t pos;
	uc debug;
	struct PList *ds;		   // #define's
	struct PList *global_vars; // global variables
	struct PList *structs;	   // Inst's of IP_DECLARE_STRUCT
};
struct Pser *new_pser(char *, uc);
struct PList *pse(struct Pser *); // instructions
struct Token *next_pser_get(struct Pser *, long);
struct Token *get_pser_token(struct Pser *, long);
#define expect(pser, t, c)                                                     \
	do {                                                                       \
		if ((t)->code != (c))                                                  \
			eet((pser)->f, (t), EXPECTED__##c, SUGGEST__##c);                  \
	} while (0)

#define consume(p) ((p)->pos++)
#define pser_cur(p) (get_pser_token((p), 0))
#define absorb(p) (next_pser_get((p), 0)) // consume + ret pser_cur
#define not_ef_and(cd, c) ((c)->code != (cd) && (c)->code != EF)
#define not_ef_and_and(cd1, cd2, c) ((c)->code != (cd1) && not_ef_and((cd2), (c))
#define match(pser, t, c)                                                      \
	do {                                                                       \
		expect(pser, t, c);                                                    \
		consume(p);                                                            \
	} while (0)

enum IP_Code {
	// directives
	IP_NONE,
	IP_EOI, // end of instructions
	// any level
	IP_ASM,
	// global level
	IP_INCLUDE,
	IP_DEFINE,
	IP_DECLARE_ENUM,
	IP_DECLARE_STRUCT,
	IP_DECLARE_FUNCTION,
	// local level
	IP_LET,

	IP_DECLARE_LABEL,
	IP_GOTO,

	IP_IF_ELIF_ELSE,
	IP_MATCH, // TODO: I_MATCH

	IP_LOOP,
	IP_FOR_LOOP,
	IP_WHILE_LOOP,
	// expression level, not decided yet
	IP_EQU,
	IP_PLUS_EQU,
	IP_MINUS_EQU,
	IP_MUL_EQU,
	IP_DIV_EQU,
	IP_SHR_EQU,
	IP_SHL_EQU,
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
	TC_FLOAT,
	// 64
	TC_VOID,
	TC_DOUBLE,
	TC_INT64,
	TC_UINT64,

	TC_PTR, // also str str if TC_UINT8 ptr
	TC_ARR,
	TC_FUN,
	TC_STRUCT,
};

int get_type_code_size(enum TypeCode);
#define size_of_type(t) (get_type_code_size((t)->code))

struct TypeWord {
	char *view;
	enum TypeCode code;
	uint32_t view_len;
};

union TypeData {
	struct TypeExpr *ptr_target;
	struct BList *struct_name;
	struct PList *args_types;
	struct PList *arr;
};

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

struct TypeExpr *get_type_expr(enum TypeCode);

struct Arg {
	struct PList *names; // PList of Tokens
	struct TypeExpr *type;
	long offset;
};

struct GlobVar {
	struct Token *name;
	struct TypeExpr *type;
	struct BList *signature;
	// also need to have value? because its compile time value
	void *value;
};

#define types_sizes_do_match(t1, t2)                                           \
	(((t1) >= TC_VOID && (t2) >= TC_VOID) ||                                   \
	 ((t1) >= TC_INT32 && (t2) >= TC_INT32 && (t1) < TC_VOID &&                \
	  (t2) < TC_VOID) ||                                                       \
	 ((t1) >= TC_INT16 && (t2) >= TC_INT16 && (t1) < TC_INT32 &&               \
	  (t2) < TC_INT32) ||                                                      \
	 ((t1) >= TC_INT8 && (t2) >= TC_INT8 && (t1) < TC_INT16 &&                 \
	  (t2) < TC_INT16))

void *expression(struct Pser *);
void parse_args(struct Pser *p, struct PList *os);
struct TypeExpr *type_expr(struct Pser *);
struct Inst *new_inst(struct Pser *, enum IP_Code, struct PList *os,
					  struct Token *);

struct Inst *get_global_inst(struct Pser *p);
enum IP_Code inst_pser_define(struct Pser *p);
enum IP_Code inst_pser_include(struct Pser *p, struct PList *os);
enum IP_Code inst_pser_asm(struct Pser *p, struct PList *os);
enum IP_Code inst_pser_enum(struct Pser *p, struct PList *os);
enum IP_Code inst_pser_struct(struct Pser *p, struct PList *os);
enum IP_Code inst_pser_dare_fun(struct Pser *p, struct PList *os);

void parse_block_of_local_inst(struct Pser *p, struct PList *os);
struct Inst *get_local_inst(struct Pser *p);

struct BList *num_to_str(long num);
struct BList *num_to_hex_str(long num);
void get_global_signature(struct GlobVar *);
int are_types_equal(struct TypeExpr *, struct TypeExpr *);

void eei(struct Fpfc *f, struct Inst *t, const char *const msg,
		 const char *const sgst);
