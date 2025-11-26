#include "gner.h"

sa(LET_8, "пусть байт ");
sa(LET_16, "пусть дбайт ");
sa(LET_32, "пусть чбайт ");
sa(LET_64, "пусть вбайт ");

sa(RAX, "рах ");
sa(RBP, "рбп ");

sa(REZERV_ZERO, "запас 0 ");
sa(SEGMENT_READ_WRITE, "участок чит изм\n\n");
sa(SEGMENT_READ_EXECUTE, "участок чит исп\n\n");
sa(LABEL_END, ":\n");
sa(ZERO_TERMINATOR, " 0\n");
sa(START_COMMENT, "\t; ");
sa(EQU, "вот ");
sa(COMM, "; ");

sa(PUSH_RBP, "толк рбп\n");
sa(MOV_RBP_RSP, "быть рбп рсп\n");
sa(MOV_MEM_RBP_OPEN, "быть (рбп ");
sa(SUB_RSP, "минс рсп ");
sa(POP_RBP, "выт рбп\n");
sa(LEAVE, "выйти\n");
sa(RET, "возд\n");
sa(STR_XOR_EAX_EAX, "искл еах еах");
sa(MOV_RAX, "быть рах ");
sa(PUSH_R15, "толк р15\n");
sa(PUSH_R14, "толк р14\n");
sa(PUSH_R13, "толк р13\n");
sa(POP_R15, "выт р15\n");
sa(POP_R14, "выт р14\n");
sa(POP_R13, "выт р13\n");

sa(BYTE, "байт ");
sa(WORD, "дбайт ");
sa(DWORD, "чбайт ");
sa(QWORD, "вбайт ");
sa(MOV, "быть ");
sa(LEA, "задр ");
sa(IMUL, "зумн ");	  // LE_BIN_MUL
sa(IDIV, "здел ");	  // LE_BIN_DIV
sa(ADD, "плюс ");	  // LE_BIN_ADD
sa(SUB, "минс ");	  // LE_BIN_SUB
sa(SHL, "сдвл ");	  // LE_BIN_SHL
sa(SHR, "сдвп ");	  // LE_BIN_SHR
sa(BIT_AND, "и ");	  // LE_BIN_BIT_AND
sa(BIT_XOR, "искл "); // LE_BIN_BIT_XOR
sa(BIT_OR, "или ");	  // LE_BIN_BIT_OR
sa(NEG, "нег ");
sa(NOT, "не ");
sa(SET0, "уст0 ");
sa(SETN0, "устн0 ");
sa(CMP, "срав ");
sa(MOV_XMM, "бытьэ ");
sa(CVTSI2SS, "пресч2со ");
sa(CVTSI2SD, "пресч2сд ");
sa(CVTSS2SD, "пресо2сд ");
sa(CVTSS2SI, "пресо2зч ");
sa(CVTSD2SI, "пресд2зч ");
sa(XCHG, "обмн ");
sa(SAL, "сдал ");
sa(SAR, "сдап ");
sa(SAL1, "сдал1 ");
sa(SAR1, "сдап1 ");
sa(SHL1, "сдвл1 ");
sa(SHR1, "сдвп1 ");
sa(TEST, "проб ");
sa(CMOVS, "сбытьз ");
sa(XOR, "искл ");
sa(MUL_SS, "умнсо ");
sa(DIV_SS, "делсо ");
sa(ADD_SS, "плюссо ");
sa(SUB_SS, "минссо ");
sa(BIT_AND_PS, "иуо ");
sa(BIT_XOR_PS, "илиуо ");
sa(BIT_OR_PS, "исклуо ");
sa(MUL_SD, "умнсд ");
sa(DIV_SD, "делсд ");
sa(ADD_SD, "плюссд ");
sa(SUB_SD, "минссд ");
sa(BIT_AND_PD, "иуд ");
sa(BIT_XOR_PD, "илиуд ");
sa(BIT_OR_PD, "исклуд ");
sa(SETB, "устн ");
sa(SETBE, "устнв ");
sa(SETA, "уств ");
sa(SETAE, "уствр ");
sa(SETL, "устм ");
sa(SETLE, "устмр ");
sa(SETG, "устб ");
sa(SETGE, "устбр ");
sa(SETE, "устр ");
sa(SETNE, "устнр ");
sa(J0, "ид0 ");
sa(JN0, "идн0 ");
sa(JB, "идн ");
sa(JBE, "иднв ");
sa(JA, "идв ");
sa(JAE, "идвр ");
sa(JL, "идм ");
sa(JLE, "идмр ");
sa(JG, "идб ");
sa(JGE, "идбр ");
sa(JE, "идр ");
sa(JNE, "иднр ");
sa(CALL, "зов ");

sa(L_PAR, "(");
sa(R_PAR, ") ");
sa(PAR_RBP, "(рбп ");
sa(OFF_RAX, "(рах) ");
sa(JMP, "идти ");
