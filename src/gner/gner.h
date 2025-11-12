#include "../prep/prep.h"
#include "regs.h"
#include <stdint.h>

struct Reg {
	struct BList *name;
	enum RegCode reg_code;
	uc size;
	uc allocated;

	uc is_value_active;
	long active_value;
	struct RegisterFamily *rf;
};

struct RegisterFamily {
	struct Reg *l; // 8 bit Lower
	struct Reg *h; // 8 bit High
	struct Reg *x; // 16 bit
	struct Reg *e; // 32 bit Extended
	struct Reg *r; // 64 bit
};

struct CPU {
	struct RegisterFamily *a;
	struct RegisterFamily *c;
	struct RegisterFamily *d;
	struct RegisterFamily *b;
	struct RegisterFamily *sp;
	struct RegisterFamily *bp;
	struct RegisterFamily *si;
	struct RegisterFamily *di;
	struct RegisterFamily *rex[8];
	struct Reg *xmm[16];
};

struct CPU *new_cpu();
void free_all_regs(struct CPU *cpu);
void free_reg_family(struct RegisterFamily *rf);
struct Reg *just_get_reg(struct CPU *cpu, enum RegCode code);

struct Reg *borrow_basic_reg(struct CPU *cpu, uc of_size);
struct Reg *try_borrow_reg(struct Token *place, struct CPU *cpu, uc of_size);
void set_value_to_reg(struct Reg *reg, long value);

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
	struct CPU *cpu;

	uint32_t indent_level;
	long stack_counter;
	struct Fggs *flags;
	struct Lbls *labels;

	struct PList *is;
	struct PList *enums; // // struct Enum's
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

	struct BList *stack_frame;
	struct BList *after_stack_frame;
	struct BList *fun_text; // fun main text

	struct BList *tmp_blist; // just tmp blist
};
#define Gg struct Gner *g

struct Gner *new_gner(struct Pser *, enum Target, uc);
void gen(struct Gner *);

#define sa(name, str)                                                          \
	const char SA_##name[] = str;                                              \
	const uint32_t SA_##name##_LEN = loa(SA_##name);
#define sae(name)                                                              \
	extern const char SA_##name[];                                             \
	extern const uint32_t SA_##name##_LEN;

// #############################################################################
// 									SA - Str Asm
// #############################################################################
sae(LET_8);
sae(LET_16);
sae(LET_32);
sae(LET_64);
sae(RAX);
sae(RBP);
sae(REZERV_ZERO);
sae(SEGMENT_READ_WRITE);
sae(SEGMENT_READ_EXECUTE);
sae(LABEL_END);
sae(ZERO_TERMINATOR);
sae(START_COMMENT);
sae(EQU);
sae(COMM);
sae(PUSH_RBP);
sae(MOV_RBP_RSP);
sae(MOV_MEM_RBP_OPEN);
sae(SUB_RSP);
sae(POP_RBP);
sae(LEAVE);
sae(RET);
sae(STR_XOR_EAX_EAX);
sae(MOV_RAX);
sae(BYTE);
sae(WORD);
sae(DWORD);
sae(QWORD);
sae(MOV);
sae(ADD);
sae(SUB);
sae(L_PAR);
sae(R_PAR);
sae(PAR_RBP);
sae(OFF_RAX);
sae(JMP);
sae(LEA);
// #############################################################################

void indent_line(struct Gner *g, struct BList *l);

#define iprint_(str, funs_name, list)                                          \
	do {                                                                       \
		indent_line(g, list);                                                  \
		blat_str_##funs_name(str);                                             \
	} while (0)
// #############################################################################
#define blat_str_gen(str) (blat(generated, (uc *)(str), (str##_LEN - 1)))
#define iprint_gen(str) iprint_(str, gen, generated)
#define print_gen(str) (blat_str_gen(str))
#define gen_add(byte) (blist_add(generated, (byte)))
#define blat_gen(list) (blat_blist(generated, (list)))
// #############################################################################
#define blat_str_bprol(str) (blat(g->bprol, (uc *)(str), (str##_LEN - 1)))
#define iprint_bprol(str) iprint_(str, bprol, g->bprol)
#define print_bprol(str) (blat_str_bprol(str))
#define bprol_add(byte) (blist_add(g->bprol, (byte)))
#define blat_bprol(list) (blat_blist(g->bprol, (list)))
// #############################################################################
#define blat_str_prol(str) (blat(g->prol, (uc *)(str), (str##_LEN - 1)))
#define iprint_prol(str) iprint_(str, prol, g->prol)
#define print_prol(str) (blat_str_prol(str))
#define prol_add(byte) (blist_add(g->prol, (byte)))
#define blat_prol(list) (blat_blist(g->prol, (list)))
// #############################################################################
#define blat_str_aprol(str) (blat(g->aprol, (uc *)(str), (str##_LEN - 1)))
#define iprint_aprol(str) iprint_(str, aprol, g->aprol)
#define print_aprol(str) (blat_str_aprol(str))
#define aprol_add(byte) (blist_add(g->aprol, (byte)))
#define blat_aprol(list) (blat_blist(g->aprol, (list)))
// #############################################################################
#define blat_str_text(str) (blat(g->text, (uc *)(str), (str##_LEN - 1)))
#define iprint_text(str) iprint_(str, text, g->text)
#define print_text(str) (blat_str_text(str))
#define text_add(byte) (blist_add(g->text, (byte)))
#define blat_text(list) (blat_blist(g->text, (list)))
// #############################################################################
// 								FUN PART
// #############################################################################
#define blat_str_stack_frame(str)                                              \
	(blat(g->stack_frame, (uc *)(str), (str##_LEN - 1)))
#define iprint_stack_frame(str) iprint_(str, stack_frame, g->stack_frame);
#define print_stack_frame(str) (blat_str_stack_frame(str))
#define stack_frame_add(byte) (blist_add(g->stack_frame, (byte)))
#define blat_stack_frame(list) (blat_blist(g->stack_frame, (list)))

#define blat_str_after_stack_frame(str)                                        \
	(blat(g->after_stack_frame, (uc *)(str), (str##_LEN - 1)))
#define iprint_after_stack_frame(str)                                          \
	iprint_(str, after_stack_frame, g->after_stack_frame)
#define print_after_stack_frame(str) (blat_str_after_stack_frame(str))
#define after_stack_frame_add(byte) (blist_add(g->after_stack_frame, (byte)))
#define blat_after_stack_frame(list) (blat_blist(g->after_stack_frame, (list)))

#define blat_str_fun_text(str) (blat(g->fun_text, (uc *)(str), (str##_LEN - 1)))
#define iprint_fun_text(str) iprint_(str, fun_text, g->fun_text)
#define print_fun_text(str) (blat_str_fun_text(str))
#define fun_text_add(byte) (blist_add(g->fun_text, (byte)))
#define blat_fun_text(list) (blat_blist(g->fun_text, (list)))

#define blat_str_ft(str) (blat(g->fun_text, (uc *)(str), (str##_LEN - 1)))
#define iprint_ft(str) iprint_(str, fun_text, g->fun_text)
#define isprint_ft(str) iprint_(SA_##str, fun_text, g->fun_text)
#define print_ft(str) (blat_str_fun_text(str))
#define sprint_ft(str) (blat_str_fun_text(SA_##str))
#define ft_add(byte) (blist_add(g->fun_text, (byte)))
#define blat_ft(list) (blat_blist(g->fun_text, (list)))
// #############################################################################

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
#define add_int_with_hex_comm(list, num)                                       \
	int_add(g->list, (num));                                                   \
	blat_str_##list(SA_START_COMMENT);                                         \
	hex_int_add(g->list, (num));                                               \
	list##_add('\n');
#define real_add(list, value)                                                  \
	do {                                                                       \
		g->tmp_blist = real_to_str((value));                                   \
		blat_blist((list), g->tmp_blist);                                      \
		blist_clear_free(g->tmp_blist);                                        \
	} while (0);

void write_fun(struct Gner *g);
void gen_linux_text(struct Gner *);

void gen_local_linux(struct Gner *g, struct Inst *in);
struct BList *gen_glob_expr_linux(struct Gner *g, struct GlobExpr *e);

struct LocalVar {
	struct Token *name;
	long stack_pointer;

	struct TypeExpr *type;
	int lvar_size;
};

struct LocalVar *new_local_var(struct Token *, struct Arg *, long);
void free_and_clear_local_vars(struct Gner *g);
struct BList *take_label(struct Gner *g, enum L_Code label_code);

void gen_local_expression_linux(struct Gner *g, struct Inst *in);
uc get_assignee_size(struct Gner *g, struct LocalExpr *e, struct GlobVar **gvar,
					 struct LocalVar **lvar);
void compare_type_and_expr(struct TypeExpr *type, struct LocalExpr *e);

struct GlobVar *find_glob_Var(struct Gner *g, struct BList *name);
struct LocalVar *find_local_Var(struct Gner *g, struct BList *name);
struct Inst *find_struct(struct BList *name);
struct BList *size_str(uc size);

void gen_dec_inc(struct Gner *g, struct LocalExpr *e, uc is_inc);

#define let_lvar_gvar struct LocalVar *lvar, struct GlobVar *gvar
#define declare_lvar_gvar                                                      \
	struct LocalVar *lvar = 0;                                                 \
	struct GlobVar *gvar = 0;
#define lvar_gvar_type() (lvar ? lvar->type : gvar->type)

void var_(struct Gner *g, let_lvar_gvar);
#define reg_(reg) blat_ft(just_get_reg(g->cpu, (reg))->name), ft_add(' ')
void sib(struct Gner *g, uc size, enum RegCode base, uc scale,
		 enum RegCode index, long disp, uc is_disp_blist);
#define sib_(size, base, scale, index, disp, is_disp_bl)                       \
	sib(g, (size), (base), (scale), (index), (disp), (is_disp_bl)), ft_add(' ')
void mov_var_(struct Gner *g, let_lvar_gvar);
void mov_reg_(Gg, enum RegCode reg);
void mov_reg_var(Gg, enum RegCode reg, let_lvar_gvar);
#define op_reg_(op, reg)                                                        \
	isprint_ft(op);                                                          \
	reg_((reg));

// ############################################################################
// 									OZER
// ############################################################################
extern constr EXPECTED_PTR_TYPE;

extern struct Gner *ogner;
struct PList *opt_local_expr(struct LocalExpr *e);
struct PList *eliminate_dead_code_from_le(struct LocalExpr *e);

void define_le_type(struct LocalExpr *e);
#define define_type_and_copy_flags_to_e(expr)                                  \
	do {                                                                       \
		define_le_type((expr));                                                \
		e->flags |= (expr)->flags;                                             \
	} while (0)
int lee(struct LocalExpr *l, struct LocalExpr *r);

int try_opt_mul(struct LocalExpr *e);
int try_opt_div(struct LocalExpr *e);
int try_opt_add_or_sub(struct LocalExpr *e);
int try_opt_shl_or_shr(struct LocalExpr *e);
int try_opt_and(struct LocalExpr *e);
int try_opt_or(struct LocalExpr *e);
int try_opt_bit_or(struct LocalExpr *e);
int try_opt_bit_and(struct LocalExpr *e);
int try_opt_more_or_less(struct LocalExpr *e);
int try_opt_eque_or_nequ(struct LocalExpr *e);
int try_opt_moree_or_lesse(struct LocalExpr *e);

void try_bin_bins(struct LocalExpr *e);
void try_bin_num_in_bin(struct LocalExpr *num,
						struct LocalExpr **root_place_in_parrent,
						enum LE_Code op_code);
void bin_l_and_r_to_e(struct LocalExpr *l, struct LocalExpr *r,
					  struct LocalExpr *e, enum LE_Code op_code);
void unary_or_bool_of_num(struct LocalExpr *e);

#define if_opted(cap, low) ((e->code == LE_BIN_##cap && try_opt_##low(e)))
#define if_opted2(cap0, cap1, low)                                             \
	(((e->code == LE_BIN_##cap0 || e->code == LE_BIN_##cap1) &&                \
	  try_opt_##low(e)))
#define update_int_view(e)                                                     \
	do {                                                                       \
		blist_clear_free((e)->tvar->view);                                     \
		(e)->tvar->view = int_to_str((e)->tvar->num);                          \
	} while (0)
#define update_real_view(e)                                                    \
	do {                                                                       \
		blist_clear_free((e)->tvar->view);                                     \
		(e)->tvar->view = real_to_str((e)->tvar->real);                        \
	} while (0)

#define cant_on_reals(op_name, op)                                             \
	constr CANT_##op_name##_ON_REALS =                                         \
		"Операция '" op "' не применима к вещественным числам.";
#define cant_on_nums(op_name, op)                                              \
	constr CANT_##op_name##_ON_NUMS =                                          \
		"Операция '" op "' не применима к числам.";
extern constr ALLOWANCE_OF_INDEXATION;
