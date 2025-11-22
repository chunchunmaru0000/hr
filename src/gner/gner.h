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

#define free_reg(reg)                                                          \
	do {                                                                       \
		(reg)->allocated = 0;                                                  \
		(reg)->is_value_active = 0;                                            \
	} while (0)

struct CPU *new_cpu();
struct Reg *just_get_reg(struct CPU *cpu, enum RegCode code);

void free_all_regs(struct CPU *cpu);
void free_reg_family(struct RegisterFamily *rf);
void free_byte_reg(struct Reg *r);

struct Reg *borrow_basic_reg(struct CPU *cpu, uc of_size);
struct Reg *borrow_xmm_reg(struct CPU *cpu);
#define is_xmm(reg) ((reg)->reg_code >= R_XMM0)
void set_value_to_reg(struct Reg *reg, long value);
struct Reg *alloc_reg_of_size(struct RegisterFamily *rf, int size);

struct Fggs {
	uc is_stack_used;

	uc is_r13_used;
	uc is_r14_used;
	uc is_r15_used;
	uc need_save_args_on_stack_count;
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
struct Reg *try_borrow_reg(struct Token *place, struct Gner *g, uc of_size);
struct Reg *try_borrow_xmm_reg(struct Token *place, struct Gner *g);
#define DO_XCHG 0
#define DO_MOV 1
void swap_basic_regs(struct Gner *g, struct RegisterFamily *rf1,
					 struct RegisterFamily *rf2, int do_mov);
struct Reg *try_alloc_reg(struct Token *tvar, struct RegisterFamily *rf,
						  int size);
#define Gg struct Gner *g

struct Gner *new_gner(struct Pser *, enum Target, uc);
void reset_flags(struct Gner *g);
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
sae(LET_8) sae(LET_16) sae(LET_32) sae(LET_64) sae(RAX) sae(RBP)
	sae(REZERV_ZERO) sae(SEGMENT_READ_WRITE) sae(SEGMENT_READ_EXECUTE) sae(
		LABEL_END) sae(ZERO_TERMINATOR) sae(START_COMMENT) sae(EQU) sae(COMM)
		sae(PUSH_RBP) sae(MOV_RBP_RSP) sae(MOV_MEM_RBP_OPEN) sae(SUB_RSP) sae(
			POP_RBP) sae(LEAVE) sae(RET) sae(STR_XOR_EAX_EAX) sae(MOV_RAX)
			sae(BYTE) sae(WORD) sae(DWORD) sae(QWORD) sae(MOV) sae(L_PAR)
				sae(R_PAR) sae(PAR_RBP) sae(OFF_RAX) sae(JMP) sae(LEA) sae(IMUL)
					sae(IDIV) sae(ADD) sae(SUB) sae(SHL) sae(SHR) sae(BIT_AND)
						sae(BIT_XOR) sae(BIT_OR) sae(NEG) sae(NOT) sae(SET0)
							sae(SETN0) sae(CMP) sae(MOV_XMM) sae(CVTSI2SS)
								sae(CVTSI2SD) sae(CVTSS2SD) sae(MUL_SS)
									sae(DIV_SS) sae(ADD_SS) sae(SUB_SS)
										sae(BIT_AND_PS) sae(BIT_XOR_PS)
											sae(BIT_OR_PS) sae(MUL_SD)
												sae(DIV_SD) sae(ADD_SD)
													sae(SUB_SD) sae(BIT_AND_PD)
														sae(BIT_XOR_PD)
															sae(BIT_OR_PD);
sae(XCHG) sae(SHL1) sae(SHR1) sae(TEST) sae(CMOVS) sae(SAL) sae(SAR) sae(SAL1)
	sae(SAR1) sae(XOR) sae(SETB) sae(SETBE) sae(SETA) sae(SETAE) sae(SETL)
		sae(SETLE) sae(SETG) sae(SETGE) sae(SETE) sae(SETNE);

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

void gen_local_expr_linux(Gg, struct LocalExpr *e);
void gen_tuple_of(Gg, struct LocalExpr *e);
void merge_tuple_of_to(struct LocalExpr *of, struct LocalExpr *to);
#define paste_with_tuple_merge(to, from)                                       \
	merge_tuple_of_to((to), (from));                                           \
	paste_le((to), (from));
#define paste_with_tuple_merge_of(to, from, of)                                \
	merge_tuple_of_to((of), (from));                                           \
	paste_le((to), (from))
void gen_local_expr_inst_linux(struct Gner *g, struct Inst *in);
uc get_assignee_size(struct Gner *g, struct LocalExpr *e, struct GlobVar **gvar,
					 struct LocalVar **lvar);
void compare_type_and_expr(struct TypeExpr *type, struct LocalExpr *e);

struct GlobVar *find_glob_Var(struct Gner *g, struct BList *name);
struct LocalVar *find_local_Var(struct Gner *g, struct BList *name);
struct Inst *find_struct(struct BList *name);
struct BList *size_str(uc size);
int le_depth(struct LocalExpr *e);

struct Reg *gen_to_reg(Gg, struct LocalExpr *e, uc of_size);
void gen_dec_inc(struct Gner *g, struct LocalExpr *e, uc is_inc);

struct Reg *after_to_reg(Gg, struct LocalExpr *e, int reg_size);
struct Reg *prime_to_reg(Gg, struct LocalExpr *e, int reg_size);
struct Reg *div_on_int(Gg, struct LocalExpr *e, struct Reg *r1);
struct Reg *div_on_mem(Gg, struct LocalExpr *e, struct Reg *r1);
struct Reg *div_on_reg(Gg, struct LocalExpr *e, struct Reg *r1, struct Reg *r2);
struct Reg *mul_on_int(Gg, struct Reg *r1, struct LocalExpr *num);
struct Reg *shift_on_int(Gg, struct LocalExpr *e, struct Reg *r1);
struct Reg *shift_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
						 struct Reg *r2);
struct Reg *cmp_on_mem_or_int(Gg, struct LocalExpr *e, struct Reg *r1,
							  struct LocalExpr *num_or_mem);
struct Reg *reverse_cmp_on_mem_or_int(Gg, struct LocalExpr *e, struct Reg *r1,
							  struct LocalExpr *num_or_mem);
struct Reg *cmp_on_reg(Gg, struct LocalExpr *e, struct Reg *r1, struct Reg *r2);

#define let_lvar_gvar struct LocalVar *lvar, struct GlobVar *gvar
#define declare_lvar_gvar                                                      \
	struct LocalVar *lvar = 0;                                                 \
	struct GlobVar *gvar = 0;
#define lvar_gvar_type() (lvar ? lvar->type : gvar->type)

void var_(struct Gner *g, let_lvar_gvar);
#define var_enter(lv, gv) var_(g, (lv), (gv)), g->fun_text->size--, ft_add('\n')
#define reg_(reg) blat_ft(just_get_reg(g->cpu, (reg))->name), ft_add(' ')
#define reg_enter(reg) blat_ft(just_get_reg(g->cpu, (reg))->name), ft_add('\n')
void sib(struct Gner *g, uc size, enum RegCode base, uc scale,
		 enum RegCode index, long disp, uc is_disp_blist);
#define sib_(size, base, scale, index, disp, is_disp_bl)                       \
	sib(g, (size), (base), (scale), (index), (long)(disp), (is_disp_bl)),      \
		ft_add(' ')
#define sib_enter(size, base, scale, index, disp, is_disp_bl)                  \
	sib(g, (size), (base), (scale), (index), (long)(disp), (is_disp_bl)),      \
		ft_add('\n')
void mov_var_(struct Gner *g, let_lvar_gvar);
void mov_reg_(Gg, enum RegCode reg);
void mov_reg_var(Gg, enum RegCode reg, let_lvar_gvar);
#define op_reg_(op, reg)                                                       \
	isprint_ft(op);                                                            \
	reg_((reg));
#define op_reg_enter(op, reg)                                                  \
	isprint_ft(op);                                                            \
	reg_enter((reg));
#define op_reg_reg(op, r1, r2)                                                 \
	isprint_ft(op);                                                            \
	reg_((r1)->reg_code);                                                      \
	reg_enter((r2)->reg_code);
struct Reg *cmp_with_int(Gg, struct LocalExpr *e, long num);
#define mov_xmm_reg_(reg)                                                      \
	isprint_ft(MOV_XMM);                                                       \
	reg_(reg);
#define mov_xmm_var_(g, lvar, gvar)                                            \
	do {                                                                       \
		isprint_ft(MOV_XMM);                                                   \
		var_((g), (lvar), (gvar));                                             \
	} while (0)
#define cvt_ss_to_sd(rcode)                                                    \
	do {                                                                       \
		isprint_ft(CVTSS2SD);                                                  \
		reg_((rcode));                                                         \
		reg_enter((rcode));                                                    \
	} while (0)
#define cmp_with_mem(reg, mem)                                                 \
	op_reg_(CMP, (reg)->reg_code);                                             \
	get_assignee_size(g, (mem), &gvar, &lvar);                                 \
	var_enter(lvar, gvar);
#define cmp_with_num(reg, number)                                              \
	op_reg_(CMP, (reg)->reg_code);                                             \
	add_int_with_hex_comm(fun_text, (number)->tvar->num);
#define cmp_with_reg(r1, r2) op_reg_reg(CMP, (r1), (r2))

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
void turn_type_to_simple(struct LocalExpr *e, enum TypeCode simple_code);

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
void opt_bin_constant_folding(struct LocalExpr *e);
void unary_or_bool_of_num(struct LocalExpr *e);

// commutative
#define is_non_commut(op)                                                      \
	(((op) == LE_BIN_DIV || (op) == LE_BIN_SUB || (op) == LE_BIN_MOD ||        \
	  (op) == LE_BIN_SHL || (op) == LE_BIN_SHR))
#define is_commut(op) ((!is_non_commut((op))))
#define is_pow_of_two(x) (((x) & ((x) - 1)) == 0)

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
