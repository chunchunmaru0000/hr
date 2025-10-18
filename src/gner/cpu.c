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
#define new_family_reg(family_reg, rcn, rsn, reg_size)                         \
	do {                                                                       \
		family_reg = malloc(sizeof(struct Reg));                               \
		family_reg->name = copy_blist_from_str(rsn);                           \
		family_reg->reg_code = R_##rcn;                                        \
		family_reg->allocated = 0;                                             \
		family_reg->is_value_active = 0;                                       \
		family_reg->active_value = 0;                                          \
	} while (0)
#define new_default_family(reg_family, rcn)                                    \
	do {                                                                       \
		cpu->reg_family = malloc(sizeof(struct RegisterFamily));               \
		new_family_reg(cpu->reg_family->l, rcn##L, STR_##rcn##L, BYTE);        \
		new_family_reg(cpu->reg_family->h, rcn##H, STR_##rcn##H, BYTE);        \
		new_family_reg(cpu->reg_family->x, rcn##X, STR_##rcn##X, WORD);        \
		new_family_reg(cpu->reg_family->e, E##rcn##X, STR_E##rcn##X, DWORD);   \
		new_family_reg(cpu->reg_family->r, R##rcn##X, STR_R##rcn##X, QWORD);   \
	} while (0)
#define new_short_family(reg_family, rcn)                                      \
	do {                                                                       \
		cpu->reg_family = malloc(sizeof(struct RegisterFamily));               \
		cpu->reg_family->l = 0;                                                \
		cpu->reg_family->h = 0;                                                \
		new_family_reg(cpu->reg_family->x, rcn, STR_##rcn, WORD);              \
		new_family_reg(cpu->reg_family->e, E##rcn, STR_E##rcn, DWORD);         \
		new_family_reg(cpu->reg_family->r, R##rcn, STR_R##rcn, QWORD);         \
	} while (0)
#define new_extended_family(num)                                               \
	do {                                                                       \
		cpu->rex[num] = malloc(sizeof(struct RegisterFamily));                 \
		new_family_reg(cpu->rex[num]->l, R##num##B, STR_R##num##B, BYTE);      \
		cpu->rex[num]->h = 0;                                                  \
		new_family_reg(cpu->rex[num]->x, R##num##W, STR_R##num##W, WORD);      \
		new_family_reg(cpu->rex[num]->e, R##num##D, STR_R##num##D, DWORD);     \
		new_family_reg(cpu->rex[num]->r, R##num, STR_R##num, QWORD);           \
	} while (0)
#define new_xmm_reg(num)                                                       \
	do {                                                                       \
		cpu->xmm[num] = malloc(sizeof(struct Reg));                            \
		new_family_reg(cpu->xmm[num], XMM##num, STR_XMM##num, XWORD);          \
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
