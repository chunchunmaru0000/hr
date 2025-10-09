#include "../prep/prep.h"
#include "../some/regs.h"
#include <stdint.h>

struct Fggs {
	uc is_stack_used;
	uc is_rbx_used;
	uc is_r12_used;
	// arguments only in registers, can be only done if there is no other args
	// in the function and no stack usage
	uc is_args_in_regs;

	uc is_data_segment_used;
};

enum L_Code {
	LC_LOOP,
	LC_WHILE,
	LC_FOR,
	LC_IF,
	LC_ELSE,
	LC_PTR,
};

struct Lbls {
	uint32_t loops;
	uint32_t whiles;
	uint32_t fors;
	uint32_t ifs;
	uint32_t elses;
	uint32_t ptrs;
};

struct Gner {
	enum Target t;
	uc debug;

	uint32_t pos;
	struct Inst *current_inst;

	uint32_t indent_level;
	long stack_counter;
	struct Fggs *flags;
	struct Lbls *labels;

	struct PList *is;
	struct PList *enums; // Defn's
	struct PList *global_vars;
	struct PList *local_vars;

	struct GlobVar *current_function;
	struct PList *local_labels; // plist of tokens with labels names

	// before prolog for defns of an assembly
	// like enums values and structs offsets
	struct BList *bprol;
	// prolog main data section
	struct BList *prol;
	// after prolog for datas that are store values by pointers
	// and also begin segment of text
	struct BList *aprol;
	// main text for all functions text and assemly
	struct BList *text;

	struct BList *fun_prol; // fun prolog
	struct BList *fun_text; // fun main text

	struct BList *tmp_blist; // just tmp blist
};

struct Gner *new_gner(struct Pser *, enum Target, uc);
void gen(struct Gner *);

// SA - Str Asm
extern const char SA_SEGMENT_READ_WRITE[];
extern const char SA_SEGMENT_READ_EXECUTE[];
extern const char SA_LABEL_END[];

extern const uint32_t SA_SEGMENT_READ_WRITE_LEN;
extern const uint32_t SA_SEGMENT_READ_EXECUTE_LEN;
extern const uint32_t SA_LABEL_END_LEN;

extern const char SA_EQU[];
extern const char SA_PUSH_RBP[];
extern const char SA_MOV_RBP_RSP[];
extern const char SA_MOV_MEM_RBP_OPEN[];
extern const char SA_START_COMMENT[];
extern const char SA_SUB_RSP[];
extern const char SA_LEAVE[];
extern const char SA_RET[];
extern const char SA_JMP[];
extern const char SA_ZERO_TERMINATOR[];

extern const uint32_t SA_EQU_LEN;
extern const uint32_t SA_PUSH_RBP_LEN;
extern const uint32_t SA_MOV_RBP_RSP_LEN;
extern const uint32_t SA_LEAVE_STACK_FRAME_LEN;
extern const uint32_t SA_MOV_MEM_RBP_OPEN_LEN;
extern const uint32_t SA_START_COMMENT_LEN;
extern const uint32_t SA_SUB_RSP_LEN;
extern const uint32_t SA_LEAVE_LEN;
extern const uint32_t SA_RET_LEN;
extern const uint32_t SA_JMP_LEN;
extern const uint32_t SA_ZERO_TERMINATOR_LEN;

extern const char SA_LET_8[];
extern const char SA_LET_16[];
extern const char SA_LET_32[];
extern const char SA_LET_64[];
extern const char SA_REZERV_ZERO[];

extern const uint32_t SA_LET_8_LEN;
extern const uint32_t SA_LET_16_LEN;
extern const uint32_t SA_LET_32_LEN;
extern const uint32_t SA_LET_64_LEN;
extern const uint32_t SA_REZERV_ZERO_LEN;

void indent_line(struct Gner *g, struct BList *l);
// #############################################################################
#define blat_str_gen(str) (blat(generated, (uc *)(str), (str##_LEN - 1)))
#define iprint_gen(str)                                                        \
	do {                                                                       \
		indent_line(g, generated);                                             \
		blat_str_gen(str);                                                     \
	} while (0)
#define print_gen(str) (blat_str_gen(str))
// #############################################################################
#define blat_str_bprol(str) (blat(g->bprol, (uc *)(str), (str##_LEN - 1)))
#define iprint_bprol(str)                                                      \
	do {                                                                       \
		indent_line(g, g->bprol);                                              \
		blat_str_bprol(str);                                                   \
	} while (0)
#define print_bprol(str) (blat_str_bprol(str))
// #############################################################################
#define blat_str_prol(str) (blat(g->prol, (uc *)(str), (str##_LEN - 1)))
#define iprint_prol(str)                                                       \
	do {                                                                       \
		indent_line(g, g->prol);                                               \
		blat_str_prol(str);                                                    \
	} while (0)
#define print_prol(str) (blat_str_prol(str))
// #############################################################################
#define blat_str_aprol(str) (blat(g->aprol, (uc *)(str), (str##_LEN - 1)))
#define iprint_aprol(str)                                                      \
	do {                                                                       \
		indent_line(g, g->aprol);                                              \
		blat_str_aprol(str);                                                   \
	} while (0)
#define print_aprol(str) (blat_str_aprol(str))
// #############################################################################
#define blat_str_text(str) (blat(g->text, (uc *)(str), (str##_LEN - 1)))
#define iprint_text(str)                                                       \
	do {                                                                       \
		indent_line(g, g->text);                                               \
		blat_str_text(str);                                                    \
	} while (0)
#define print_text(str) (blat_str_text(str))
// #############################################################################
#define blat_str_fun_prol(str) (blat(g->fun_prol, (uc *)(str), (str##_LEN - 1)))
#define iprint_fun_prol(str)                                                   \
	do {                                                                       \
		indent_line(g, g->fun_prol);                                           \
		blat_str_fun_prol(str);                                                \
	} while (0)
#define print_fun_prol(str) (blat_str_fun_prol(str))
// #############################################################################
#define blat_str_fun_text(str) (blat(g->fun_text, (uc *)(str), (str##_LEN - 1)))
#define iprint_fun_text(str)                                                   \
	do {                                                                       \
		indent_line(g, g->fun_text);                                           \
		blat_str_fun_text(str);                                                \
	} while (0)
#define print_fun_text(str) (blat_str_fun_text(str))
// #############################################################################

#define bprol_add(byte) (blist_add(g->bprol, (byte)))
#define prol_add(byte) (blist_add(g->prol, (byte)))
#define aprol_add(byte) (blist_add(g->aprol, (byte)))
#define text_add(byte) (blist_add(g->text, (byte)))
#define gen_add(byte) (blist_add(generated, (byte)))

#define fun_prol_add(byte) (blist_add(g->fun_prol, (byte)))
#define fun_text_add(byte) (blist_add(g->fun_text, (byte)))

#define int_add(list, value)                                                   \
	do {                                                                       \
		g->tmp_blist = int_to_str((value));                                    \
		blat_blist((list), g->tmp_blist);                                      \
		blist_clear_free(g->tmp_blist);                                        \
	} while (0);
#define hex_int_add(list, value)                                               \
	do {                                                                       \
		g->tmp_blist = int_to_hex_str((value));                                \
		blat_blist((list), g->tmp_blist);                                      \
		blist_clear_free(g->tmp_blist);                                        \
	} while (0);
#define real_add(list, value)                                                  \
	do {                                                                       \
		g->tmp_blist = real_to_str((value));                                   \
		blat_blist((list), g->tmp_blist);                                      \
		blist_clear_free(g->tmp_blist);                                        \
	} while (0);

void write_fun(struct Gner *g);
void gen_Асм_Linux_64_text(struct Gner *);

void gen_local_Асм_Linux_64(struct Gner *g, struct Inst *in);
struct BList *gen_glob_expr_Асм_Linux_64(struct Gner *g, struct GlobExpr *e);

struct LocalVar {
	struct Token *name;
	long stack_pointer;

	struct TypeExpr *type;
	int lvar_size;
};

struct LocalVar *new_local_var(struct Token *, struct Arg *, long);
void free_and_clear_local_vars(struct Gner *g);
struct BList *take_label(struct Gner *g, enum L_Code label_code);

void gen_local_expression_Асм_linux_x64(struct Gner *g, struct Inst *in);
