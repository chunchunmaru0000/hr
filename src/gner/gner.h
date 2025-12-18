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
	// TODO: struct RegisterFamily *wanna_to
	// used when try borrow, may be used to gen in prefered register
};

#define free_reg(reg)                                                          \
	do {                                                                       \
		(reg)->allocated = 0;                                                  \
		(reg)->is_value_active = 0;                                            \
	} while (0)
#define reg_of_sz(rf, sz)                                                      \
	(sz) == BYTE	? (rf)->l                                                  \
	: (sz) == WORD	? (rf)->x                                                  \
	: (sz) == DWORD ? (rf)->e                                                  \
					: (rf)->r
#define as_rfs(cpu) ((struct RegisterFamily **)(cpu))

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
extern uc function_body_return;

struct Gner {
	enum Target t;
	uc debug;

	uint32_t pos;
	struct Inst *current_inst;
	struct CPU *cpu;

	uint32_t indent_level;
	long stack_counter;
	struct Fggs *flags;

	struct PList *is;
	struct PList *enums; // // struct Enum's
	struct PList *global_vars;
	struct PList *local_vars;
	struct PList *same_name_funs; // SameNameFuns's

	struct GlobVar *current_function;
	struct BList *label_to_ret;
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
void save_allocated_regs(Gg, struct Token *place);
void get_reg_to_rf(struct Token *tvar, Gg, struct Reg *reg,
				   struct RegisterFamily *rf);
struct Reg *get_reg_to_size(Gg, struct Reg *r, int wanna_size);

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
		sae(SETLE) sae(SETG) sae(SETGE) sae(SETE) sae(SETNE) sae(J0) sae(JN0)
			sae(JB) sae(JBE) sae(JA) sae(JAE) sae(JL) sae(JLE) sae(JG) sae(JGE)
				sae(JE) sae(JNE) sae(CALL) sae(CVTSS2SI) sae(CVTSD2SI)
					sae(PUSH_R15) sae(PUSH_R14) sae(PUSH_R13) sae(POP_R15)
						sae(POP_R14) sae(POP_R13) sae(MEM_PLUS) sae(CBW)
							sae(CWDE) sae(CDQE) sae(CWD) sae(CDQ) sae(CQO)
								sae(INC) sae(DEC);

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
// TODO: i propably lose mem here if use with size_str(...)
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
#define real_add_enter(list, value)                                            \
	real_add(g->list, value);                                                  \
	list##_add('\n')

void write_fun(struct Gner *g);
void gen_linux_text(struct Gner *);

void gen_opted(Gg, struct LocalExpr *e);
void gen_block(Gg, struct PList *os);
void gen_local_linux(struct Gner *g, struct Inst *in);
struct BList *gen_glob_expr_linux(struct Gner *g, struct GlobExpr *e);
void write_flags_and_end_stack_frame(Gg);

struct LocalVar {
	struct Token *name;
	long stack_pointer;

	struct TypeExpr *type;
	int lvar_size;
};

struct LocalVar *new_local_var(struct Token *, struct Arg *, long);
void free_and_clear_local_vars(struct Gner *g);

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
void iprint_jmp(Gg, enum LE_Code le, int is_u);

struct Reg *gen_to_reg(Gg, struct LocalExpr *e, uc of_size);
void gen_dec_inc(struct Gner *g, struct LocalExpr *e, uc is_inc);
void gen_assign(struct Gner *g, struct LocalExpr *e);
void gen_call(Gg, struct LocalExpr *e);
void gen_terry(Gg, struct LocalExpr *e);

struct Reg *after_to_reg(Gg, struct LocalExpr *e, int reg_size);
struct Reg *prime_to_reg(Gg, struct LocalExpr *e, int reg_size);
struct Reg *div_on_int(Gg, struct LocalExpr *e, struct Reg *r1);
struct Reg *div_on_mem(Gg, struct LocalExpr *e, struct Reg *r1);
struct Reg *div_on_reg(Gg, struct LocalExpr *e, struct Reg *r1, struct Reg *r2);
struct Reg *mod_on_int(Gg, struct LocalExpr *e, struct Reg *r1);
struct Reg *mod_on_mem(Gg, struct LocalExpr *e, struct Reg *r1);
struct Reg *mod_on_reg(Gg, struct LocalExpr *e, struct Reg *r1, struct Reg *r2);
struct Reg *mul_on_int(Gg, struct Reg *r1, long mul_on);
struct Reg *shift_on_int(Gg, struct LocalExpr *e, struct Reg *r1);
struct Reg *shift_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
						 struct Reg *r2);
struct Reg *and_to_reg(Gg, struct LocalExpr *e);
struct Reg *or_to_reg(Gg, struct LocalExpr *e);
struct Reg *cvt_from_xmm(Gg, struct LocalExpr *e, struct Reg *xmm_reg);
struct Reg *unary_dec_inc(Gg, struct LocalExpr *e, uc is_inc);
struct Reg *after_dec_inc(Gg, struct LocalExpr *e, uc is_inc);
struct Reg *call_to_reg(Gg, struct LocalExpr *e);
struct Reg *terry_to_reg(Gg, struct LocalExpr *e);
struct Reg *assign_to_reg(Gg, struct LocalExpr *e);

struct Reg *cmp_with_set(Gg, struct LocalExpr *e);
void cmp_bool(Gg, struct LocalExpr *e);
void just_cmp(Gg, struct LocalExpr *e);
void and_cmp(Gg, struct LocalExpr *e, struct BList *false_label);
void or_cmp(Gg, struct LocalExpr *e, struct BList *true_label);

#define get_regs_to_one_size(r1p, r2p)                                         \
	if ((*(r1p))->size > (*(r2p))->size)                                       \
		*(r2p) = get_reg_to_size(g, *(r2p), (*(r1p))->size);                   \
	else if ((*(r2p))->size > (*(r1p))->size)                                  \
		*(r1p) = get_reg_to_size(g, *(r1p), (*(r2p))->size);
#define declare_lvar_gvar                                                      \
	struct LocalVar *lvar = 0;                                                 \
	struct GlobVar *gvar = 0;
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
#define op_ isprint_ft
#define op_reg_(op, reg)                                                       \
	op_(op);                                                                   \
	reg_((reg));
#define op_reg_enter(op, reg)                                                  \
	op_(op);                                                                   \
	reg_enter((reg));
#define op_reg_reg(op, r1, r2)                                                 \
	op_(op);                                                                   \
	reg_((r1)->reg_code);                                                      \
	reg_enter((r2)->reg_code);
#define mov_xmm_reg_(reg)                                                      \
	op_(MOV_XMM);                                                              \
	reg_(reg);
#define cvt_ss_to_sd(rcode)                                                    \
	do {                                                                       \
		op_(CVTSS2SD);                                                         \
		reg_((rcode));                                                         \
		reg_enter((rcode));                                                    \
	} while (0)
#define add_label(label)                                                       \
	do {                                                                       \
		indent_line(g, g->fun_text);                                           \
		blat_ft(label), ft_add(':'), ft_add('\n');                             \
	} while (0)
#define reverse_cmp_le(le)                                                     \
	((le) == LE_BIN_LESS	? LE_BIN_MOREE                                     \
	 : (le) == LE_BIN_LESSE ? LE_BIN_MORE                                      \
	 : (le) == LE_BIN_MORE	? LE_BIN_LESSE                                     \
	 : (le) == LE_BIN_MOREE ? LE_BIN_LESS                                      \
							: (le))
#define opposite_cmp_le(le)                                                    \
	((le) == LE_BIN_LESS		 ? LE_BIN_MOREE                                \
	 : (le) == LE_BIN_LESSE		 ? LE_BIN_MORE                                 \
	 : (le) == LE_BIN_MORE		 ? LE_BIN_LESSE                                \
	 : (le) == LE_BIN_MOREE		 ? LE_BIN_LESS                                 \
	 : (le) == LE_BIN_EQUALS	 ? LE_BIN_NOT_EQUALS                           \
	 : (le) == LE_BIN_NOT_EQUALS ? LE_BIN_EQUALS                               \
								 : (le))
#define free_reg_or_rf_if_not_zero(r)                                          \
	if ((r)) {                                                                 \
		if (is_xmm((r)))                                                       \
			free_reg((r));                                                     \
		else                                                                   \
			free_reg_family((r)->rf);                                          \
	}
int is_mem(struct LocalExpr *e);
void inner_mem(Gg, struct LocalExpr *e);
void mem_(Gg, struct LocalExpr *e, int of_size);
#define mem_enter(e, sz) mem_(g, (e), (sz)), g->fun_text->size--, ft_add('\n')
void gen_mem_tuple(Gg, struct LocalExpr *e);
#define op_mem_(op, e, sz)                                                     \
	op_(op);                                                                   \
	mem_(g, (e), (sz));
struct LocalExpr *is_not_assignable_or_trailed(struct LocalExpr *e);
struct Reg *last_inner_mem(Gg, struct LocalExpr *e, struct LocalExpr *trailed,
						   struct BList *imt);
struct Reg *gen_to_reg_with_last_mem(Gg, struct LocalExpr *e,
									 struct LocalExpr *trailed,
									 struct BList **last_mem_str);
#define last_mem_enter(lm_str)                                                 \
	blat_ft((lm_str)), g->fun_text->size--, ft_add('\n')
#define op_last_mem_(op, lm)                                                   \
	op_(op);                                                                   \
	blat_ft((lm));
extern int lm_size;
#define jmp_(l)                                                                \
	op_(JMP);                                                                  \
	blat_ft((l)), ft_add('\n');

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
extern constr NOT_ASSIGNABLE;
