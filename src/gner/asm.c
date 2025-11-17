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

sa(BYTE, "байт ");
sa(WORD, "дбайт ");
sa(DWORD, "чбайт ");
sa(QWORD, "вбайт ");
sa(MOV, "быть ");
sa(LEA, "задр ");
sa(MUL, "зумн ");	  // LE_BIN_MUL
sa(DIV, "здел ");	  // LE_BIN_DIV
sa(ADD, "плюс ");	  // LE_BIN_ADD
sa(SUB, "минс ");	  // LE_BIN_SUB
sa(SHL, "сдвл ");	  // LE_BIN_SHL
sa(SHR, "сдвп ");	  // LE_BIN_SHR
sa(BIT_AND, "и ");	  // LE_BIN_BIT_AND
sa(BIT_XOR, "искл "); // LE_BIN_BIT_XOR
sa(BIT_OR, "или ");	  // LE_BIN_BIT_OR
sa(NEG, "нег ");
sa(NOT, "не ");
sa(SETE, "уст0 ");
sa(SETNE, "устн0 ");
sa(CMP, "срав ");
sa(MOV_XMM, "бытьэ ");

sa(L_PAR, "(");
sa(R_PAR, ") ");
sa(PAR_RBP, "(рбп ");
sa(OFF_RAX, "(рах) ");
sa(JMP, "идти ");
