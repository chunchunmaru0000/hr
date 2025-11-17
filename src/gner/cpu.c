#include "gner.h"
#include <stdlib.h>

char *const STR_AL = "ал";
char *const STR_CL = "сл";
char *const STR_DL = "дл";
char *const STR_BL = "бд";
char *const STR_AH = "аг";
char *const STR_CH = "сг";
char *const STR_DH = "дг";
char *const STR_BH = "бг";
char *const STR_R8B = "б8";
char *const STR_R9B = "б9";
char *const STR_R10B = "б10";
char *const STR_R11B = "б11";
char *const STR_R12B = "б12";
char *const STR_R13B = "б13";
char *const STR_R14B = "б14";
char *const STR_R15B = "б15";
char *const STR_AX = "ах";
char *const STR_CX = "сх";
char *const STR_DX = "дх";
char *const STR_BX = "бх";
char *const STR_SP = "сп";
char *const STR_BP = "бп";
char *const STR_SI = "си";
char *const STR_DI = "ди";
char *const STR_R8W = "д8";
char *const STR_R9W = "д9";
char *const STR_R10W = "д10";
char *const STR_R11W = "д11";
char *const STR_R12W = "д12";
char *const STR_R13W = "д13";
char *const STR_R14W = "д14";
char *const STR_R15W = "д15";
char *const STR_EAX = "еах";
char *const STR_ECX = "есх";
char *const STR_EDX = "едх";
char *const STR_EBX = "ебх";
char *const STR_ESP = "есп";
char *const STR_EBP = "ебп";
char *const STR_ESI = "еси";
char *const STR_EDI = "еди";
char *const STR_R8D = "е8";
char *const STR_R9D = "е9";
char *const STR_R10D = "е10";
char *const STR_R11D = "е11";
char *const STR_R12D = "е12";
char *const STR_R13D = "е13";
char *const STR_R14D = "е14";
char *const STR_R15D = "е15";
char *const STR_RAX = "рах";
char *const STR_RCX = "рсх";
char *const STR_RDX = "рдх";
char *const STR_RBX = "рбх";
char *const STR_RSP = "рсп";
char *const STR_RBP = "рбп";
char *const STR_RSI = "рси";
char *const STR_RDI = "рди";
char *const STR_R8 = "р8";
char *const STR_R9 = "р9";
char *const STR_R10 = "р10";
char *const STR_R11 = "р11";
char *const STR_R12 = "р12";
char *const STR_R13 = "р13";
char *const STR_R14 = "р14";
char *const STR_R15 = "р15";
char *const STR_XMM0 = "э0";
char *const STR_XMM1 = "э1";
char *const STR_XMM2 = "э2";
char *const STR_XMM3 = "э3";
char *const STR_XMM4 = "э4";
char *const STR_XMM5 = "э5";
char *const STR_XMM6 = "э6";
char *const STR_XMM7 = "э7";
char *const STR_XMM8 = "э8";
char *const STR_XMM9 = "э9";
char *const STR_XMM10 = "э10";
char *const STR_XMM11 = "э11";
char *const STR_XMM12 = "э12";
char *const STR_XMM13 = "э13";
char *const STR_XMM14 = "э14";
char *const STR_XMM15 = "э15";

// rcn - regster capital name
// rsn - regster string name
#define new_family_reg(family_reg, rcn, rsn, reg_size, rfm)                    \
	do {                                                                       \
		(family_reg) = malloc(sizeof(struct Reg));                             \
		(family_reg)->name = copy_blist_from_str((rsn));                       \
		(family_reg)->reg_code = R_##rcn;                                      \
		(family_reg)->size = (reg_size);                                       \
		free_reg(family_reg);                                                  \
		(family_reg)->active_value = 0;                                        \
		(family_reg)->rf = (rfm);                                              \
	} while (0)
#define new_default_family(rf, rcn)                                            \
	do {                                                                       \
		cpu->rf = malloc(sizeof(struct RegisterFamily));                       \
		new_family_reg(cpu->rf->l, rcn##L, STR_##rcn##L, BYTE, cpu->rf);       \
		new_family_reg(cpu->rf->h, rcn##H, STR_##rcn##H, BYTE, cpu->rf);       \
		new_family_reg(cpu->rf->x, rcn##X, STR_##rcn##X, WORD, cpu->rf);       \
		new_family_reg(cpu->rf->e, E##rcn##X, STR_E##rcn##X, DWORD, cpu->rf);  \
		new_family_reg(cpu->rf->r, R##rcn##X, STR_R##rcn##X, QWORD, cpu->rf);  \
	} while (0)
#define new_short_family(rf, rcn)                                              \
	do {                                                                       \
		cpu->rf = malloc(sizeof(struct RegisterFamily));                       \
		cpu->rf->l = 0;                                                        \
		cpu->rf->h = 0;                                                        \
		new_family_reg(cpu->rf->x, rcn, STR_##rcn, WORD, cpu->rf);             \
		new_family_reg(cpu->rf->e, E##rcn, STR_E##rcn, DWORD, cpu->rf);        \
		new_family_reg(cpu->rf->r, R##rcn, STR_R##rcn, QWORD, cpu->rf);        \
	} while (0)
#define new_extended_family(num)                                               \
	do {                                                                       \
		cpu->rex[num - 8] = malloc(sizeof(struct RegisterFamily));             \
		new_family_reg(cpu->rex[num - 8]->l, R##num##B, STR_R##num##B, BYTE,   \
					   cpu->rex[num - 8]);                                     \
		cpu->rex[num - 8]->h = 0;                                              \
		new_family_reg(cpu->rex[num - 8]->x, R##num##W, STR_R##num##W, WORD,   \
					   cpu->rex[num - 8]);                                     \
		new_family_reg(cpu->rex[num - 8]->e, R##num##D, STR_R##num##D, DWORD,  \
					   cpu->rex[num - 8]);                                     \
		new_family_reg(cpu->rex[num - 8]->r, R##num, STR_R##num, QWORD,        \
					   cpu->rex[num - 8]);                                     \
	} while (0)
#define new_xmm_reg(num)                                                       \
	do {                                                                       \
		cpu->xmm[num] = malloc(sizeof(struct Reg));                            \
		new_family_reg(cpu->xmm[num], XMM##num, STR_XMM##num, XWORD, 0);       \
	} while (0)

struct CPU *new_cpu() {
	struct CPU *cpu = malloc(sizeof(struct CPU));

	new_default_family(a, A);
	new_default_family(c, C);
	new_default_family(d, D);
	new_default_family(b, B);
	new_short_family(sp, SP);
	new_short_family(bp, BP);
	new_short_family(si, SI);
	new_short_family(di, DI);
	new_extended_family(8);
	new_extended_family(9);
	new_extended_family(10);
	new_extended_family(11);
	new_extended_family(12);
	new_extended_family(13);
	new_extended_family(14);
	new_extended_family(15);
	new_xmm_reg(0);
	new_xmm_reg(1);
	new_xmm_reg(2);
	new_xmm_reg(3);
	new_xmm_reg(4);
	new_xmm_reg(5);
	new_xmm_reg(6);
	new_xmm_reg(7);
	new_xmm_reg(8);
	new_xmm_reg(9);
	new_xmm_reg(10);
	new_xmm_reg(11);
	new_xmm_reg(12);
	new_xmm_reg(13);
	new_xmm_reg(14);
	new_xmm_reg(15);
	return cpu;
}

void free_reg_family(struct RegisterFamily *rf) {
	free_reg(rf->r);
	free_reg(rf->e);
	free_reg(rf->x);
	if (rf->h)
		free_reg(rf->h);
	if (rf->l)
		free_reg(rf->l);
}

void free_byte_reg(struct Reg *r) {
	struct RegisterFamily *rf = r->rf;
	if (r == rf->h)
		free_reg(rf->h);
	else if (r == rf->l)
		free_reg(rf->l);

	if ((rf->h && rf->h->allocated) || (rf->l && rf->l->allocated))
		return;
	free_reg(rf->e);
	free_reg(rf->x);
	free_reg(rf->r);
}

#define as_rfs(cpu) ((struct RegisterFamily **)(cpu))
#define r_code(reg) ((reg)->r->reg_code)

void free_all_regs(struct CPU *cpu) {
	struct RegisterFamily **rfs;
	struct Reg **xmm_regs;
	u32 i;

	for (i = 0, rfs = as_rfs(cpu); i < 16; i++, rfs++)
		free_reg_family(*rfs);

	for (i = 0, xmm_regs = (struct Reg **)rfs; i < 16; i++, xmm_regs++)
		free_reg(*xmm_regs);
}

#define alloc_reg(reg)                                                         \
	do {                                                                       \
		(reg)->allocated = 1;                                                  \
		(reg)->is_value_active = 0;                                            \
	} while (0)
#define alloc_all_family_reg(rf)                                               \
	do {                                                                       \
		alloc_reg((rf)->r);                                                    \
		alloc_reg((rf)->e);                                                    \
		alloc_reg((rf)->x);                                                    \
		if ((rf)->h)                                                           \
			alloc_reg((rf)->h);                                                \
		if ((rf)->l)                                                           \
			alloc_reg((rf)->l);                                                \
	} while (0)

struct Reg *borrow_basic_reg(struct CPU *cpu, uc of_size) {
	struct RegisterFamily **rfs = as_rfs(cpu);
	struct RegisterFamily *rf;
	u32 i;

	for (i = 0, rfs = as_rfs(cpu); i < 16; i++, rfs++) {
		rf = *rfs;
		if (r_code(rf) == R_RBP || r_code(rf) == R_RSP)
			continue;

		if (of_size == BYTE) {
			if (rf->l && rf->l->size == of_size && !rf->l->allocated) {
				rf->r->allocated = 1;
				rf->e->allocated = 1;
				rf->x->allocated = 1;

				rf->l->allocated = 1;
				return rf->l;
			}
			if (rf->h && rf->h->size == of_size && !rf->h->allocated) {
				rf->r->allocated = 1;
				rf->e->allocated = 1;
				rf->x->allocated = 1;

				rf->h->allocated = 1;
				return rf->h;
			}
		}
		if (of_size == WORD && rf->x->size == of_size && !rf->x->allocated) {
			alloc_all_family_reg(rf);
			return rf->x;
		}
		if (of_size == DWORD && rf->e->size == of_size && !rf->e->allocated) {
			alloc_all_family_reg(rf);
			return rf->e;
		}
		if (of_size == QWORD && rf->r->size == of_size && !rf->r->allocated) {
			alloc_all_family_reg(rf);
			return rf->r;
		}
	}

	return 0;
}

void set_value_to_reg(struct Reg *reg, long value) {
	reg->is_value_active = 1;
	reg->active_value = value;
}

struct Reg *borrow_xmm_reg(struct CPU *cpu) {
	struct Reg **xmms = cpu->xmm;
	struct Reg *xmm;
	u32 i;
	for (i = 0, xmms = cpu->xmm; i < 16; i++, xmms++)
		if (!(xmm = *xmms)->allocated) {
			xmm->allocated = 1;
			return xmm;
		}
	return 0;
}

struct Reg *just_get_reg(struct CPU *cpu, enum RegCode code) {
	struct RegisterFamily *rf;

	if (code < R_XMM0) {
		rf = as_rfs(cpu)[(code - 1) % 16];
		return is_r64(code)	  ? rf->r
			   : is_r32(code) ? rf->e
			   : is_r8h(code) ? rf->h
			   : is_r8(code)  ? rf->l
			   : is_r8h(code) ? rf->h
							  : rf->x;
	}
	return cpu->xmm[code - R_XMM0];
}
