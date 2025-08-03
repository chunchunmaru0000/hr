#include "gner.h"
#include <stdio.h>

uint32_t put_args_on_the_stack_Асм_Linux_64(struct Gner *g, struct Inst *in);
void declare_struct_arg(struct Gner *g, struct Token *strct, struct Arg *arg);

const char SA_SEGMENT_READ_WRITE[] = "участок чит изм\n\n";
const char SA_SEGMENT_READ_EXECUTE[] = "участок чит исп\n\n";
const char SA_LABEL_END[] = ":\n";

const uint32_t SA_SEGMENT_READ_WRITE_LEN = loa(SA_SEGMENT_READ_WRITE);
const uint32_t SA_SEGMENT_READ_EXECUTE_LEN = loa(SA_SEGMENT_READ_EXECUTE);
const uint32_t SA_LABEL_END_LEN = loa(SA_LABEL_END);

const char SA_EQU[] = "вот ";
const char SA_PUSH_RBP[] = "толк рбп\n";
const char SA_MOV_RBP_RSP[] = "быть рбп рсп\n";
const char SA_MOV_MEM_RBP_OPEN[] = "быть (рбп ";
const char SA_START_COMMENT[] = "\t; ";
const char SA_SUB_RSP[] = "минс рсп ";
const char SA_LEAVE[] = "выйти\n";
const char SA_RET[] = "возд\n";

const uint32_t SA_EQU_LEN = loa(SA_EQU);
const uint32_t SA_PUSH_RBP_LEN = loa(SA_PUSH_RBP);
const uint32_t SA_MOV_RBP_RSP_LEN = loa(SA_MOV_RBP_RSP);
const uint32_t SA_MOV_MEM_RBP_OPEN_LEN = loa(SA_MOV_MEM_RBP_OPEN);
const uint32_t SA_START_COMMENT_LEN = loa(SA_START_COMMENT);
const uint32_t SA_SUB_RSP_LEN = loa(SA_SUB_RSP);
const uint32_t SA_LEAVE_LEN = loa(SA_LEAVE);
const uint32_t SA_RET_LEN = loa(SA_RET);

const struct Register regs[] = {
	{"р8", 3, R_R8, QWORD},	  {"р9", 3, R_R9, QWORD},
	{"р10", 3, R_R10, QWORD}, {"р11", 3, R_R11, QWORD},
	{"р12", 3, R_R12, QWORD}, {"р13", 3, R_R13, QWORD},
	{"р14", 3, R_R14, QWORD}, {"р15", 3, R_R15, QWORD},
};

const struct Register argument_regs_BYTE[7] = {
	{"б8", 3, R_R8B, BYTE},	  {"б9", 3, R_R9B, BYTE},
	{"б10", 4, R_R10B, BYTE}, {"б11", 4, R_R11B, BYTE},
	{"б13", 4, R_R13B, BYTE}, {"б14", 4, R_R14B, BYTE},
	{"б15", 4, R_R15B, BYTE},
};
const struct Register argument_regs_WORD[7] = {
	{"д8", 3, R_R8W, WORD},	  {"д9", 3, R_R9W, WORD},
	{"д10", 4, R_R10W, WORD}, {"д11", 4, R_R11W, WORD},
	{"д13", 4, R_R13W, WORD}, {"д14", 4, R_R14W, WORD},
	{"д15", 4, R_R15W, WORD},
};
const struct Register argument_regs_DWORD[7] = {
	{"е8", 3, R_R8D, DWORD},   {"е9", 3, R_R9D, DWORD},
	{"е10", 4, R_R10D, DWORD}, {"е11", 4, R_R11D, DWORD},
	{"е13", 4, R_R13D, DWORD}, {"е14", 4, R_R14D, DWORD},
	{"е15", 4, R_R15D, DWORD},
};
const struct Register argument_regs_QWORD[7] = {
	{"р8", 3, R_R8, QWORD},	  {"р9", 3, R_R9, QWORD},
	{"р10", 4, R_R10, QWORD}, {"р11", 4, R_R11, QWORD},
	{"р13", 4, R_R13, QWORD}, {"р14", 4, R_R14, QWORD},
	{"р15", 4, R_R15, QWORD},
};

void gen_Асм_Linux_64_text(struct Gner *g) {
	uint32_t i, j, local_i;
	struct Inst *in;
	struct Token *tok, *tok2;
	struct GlobVar *global_var;
	struct Defn *defn;

	g->flags->is_data_segment_used = 0;

	for (i = 0; i < g->is->size; i++) {
		in = plist_get(g->is, i);
		g->pos = i;

		switch (in->code) {
		case IP_EOI:
			goto end_gen_Асм_text_loop;
		case IP_ASM:
			// ### os explanation:
			//   _ - assembly string token

			tok = plist_get(in->os, 0);
			blat_blist(g->text, tok->str);
			break;
		case IP_DECLARE_ENUM:
			// ### os explanation:
			//   _ - name
			// ... - defns where name is name and value is num

			for (j = 1; j < in->os->size; j++) {
				defn = plist_get(in->os, j);
				iprint_bprol(SA_EQU); // вот
				blat_blist(g->bprol, defn->view);
				bprol_add(' ');
				int_add(g->bprol, (long)defn->value);
				bprol_add('\n');
			}

			bprol_add('\n');
			break;
		case IP_DECLARE_STRUCT:
			// ### os explanation:
			//   _ - name
			//   _ - size
			// ... - fields that are Arg's

			tok = plist_get(in->os, 0);

			for (j = 2; j < in->os->size; j++)
				declare_struct_arg(g, tok, plist_get(in->os, j));

			bprol_add('\n');
			break;
		case IP_DECLARE_FUNCTION:
			// ### os explanation:
			//   _ - GlobVar with name and type
			// ... - Arg's
			//   0 - zero terminator
			// ... - local inctrustions

			// reset things before
			g->stack_counter = 0;
			free_and_clear_local_vars(g);
			plist_clear(g->local_labels);
			g->current_function = plist_get(in->os, 0);
			// reset flags
			g->flags->is_stack_used = 0;
			g->flags->is_rbx_used = 0;
			g->flags->is_r12_used = 0;
			g->flags->is_args_in_regs = 1;
			// begin stack frame
			global_var = plist_get(in->os, 0);

			blat_blist(g->fun_prol, global_var->signature); // fun label
			blat_str_fun_prol(SA_LABEL_END);				// :
			g->indent_level++;
			iprint_fun_prol(SA_PUSH_RBP);
			iprint_fun_prol(SA_MOV_RBP_RSP);

			local_i = put_args_on_the_stack_Асм_Linux_64(g, in);
			// function body
			for (; local_i < in->os->size; local_i++)
				gen_local_Асм_Linux_64(g, plist_get(in->os, local_i));
			// free stack in return statement

			// leave also does pop rbp so its needed anyway
			iprint_fun_text(SA_LEAVE);
			iprint_fun_text(SA_RET);
			fun_text_add('\n');
			g->indent_level--;

			// reset things after
			g->current_function = 0;

			write_fun(g);
			break;
		case IP_LET:
			// ### os explanation:
			// ... - GlobVar's

			if (!g->flags->is_data_segment_used) {
				iprint_prol(SA_SEGMENT_READ_WRITE);
				g->flags->is_data_segment_used = 1;
			}

			for (j = 0; j < in->os->size; j++) {
				global_var = plist_get(in->os, j);

				blat_blist(g->prol, global_var->signature);
				blat_str_prol(SA_LABEL_END); // :
			}

			g->indent_level++;
			gen_glob_expr_Асм_Linux_64(g, global_var);
			g->indent_level--;
			break;
		default:
			eei(in, "эээ", 0);
		}
	}
end_gen_Асм_text_loop:;

	prol_add('\n');
	iprint_prol(SA_SEGMENT_READ_EXECUTE);
}

void declare_struct_arg(struct Gner *g, struct Token *strct, struct Arg *arg) {
	struct Token *name;

	for (uint32_t i = 0; i < arg->names->size; i++) {
		iprint_bprol(SA_EQU);

		blat_blist(g->bprol, strct->view);
		name = plist_get(arg->names, i);
		bprol_add('.');
		blat_blist(g->bprol, name->view);

		bprol_add('\t');
		int_add(g->bprol, arg->offset);
		blat_str_bprol(SA_START_COMMENT);
		hex_int_add(g->bprol, arg->offset);

		bprol_add('\n');
	}
}

// фц взять_регстры_размера(размер:ч32) [лик Регистр 7]
const struct Register *get_regs_of_size(int size_of_var, int i) {
	switch (size_of_var) {
	case BYTE:
		return argument_regs_BYTE + i;
	case WORD:
		return argument_regs_WORD + i;
	case DWORD:
		return argument_regs_DWORD + i;
	case QWORD:
		return argument_regs_QWORD + i;
	}
	printf("\t\t\tЭЭЭ ЭЭЭ ЭЭЭ\n");
	exit(1);
}

uint32_t put_args_on_the_stack_Асм_Linux_64(struct Gner *g, struct Inst *in) {
	// fun in->os are: fun variable, args..., 0 term, instructions...
	uint32_t i, j, mem_counter = 0;
	struct Arg *arg = plist_get(in->os, 1);
	struct LocalVar *var;
	const struct Register *reg;
	long last_offset = -1;

	for (i = 2; arg; i++) {
		if (arg->offset != last_offset)
			g->stack_counter -= arg->arg_size;

		for (j = 0; j < arg->names->size; j++) {
			var =
				new_local_var(plist_get(arg->names, j), arg, g->stack_counter);
			plist_add(g->local_vars, var);

			iprint_fun_prol(SA_EQU);				  // вот
			blat_blist(g->fun_prol, var->name->view); // name
			fun_prol_add(' ');
			int_add(g->fun_prol, g->stack_counter);
			blat_str_fun_prol(SA_START_COMMENT); // \t;
			hex_int_add(g->fun_prol, g->stack_counter);
			fun_prol_add('\n');
		}

		if (arg->offset != last_offset) {
			// быть (рсп - g->tmp_blist) register
			iprint_fun_prol(SA_MOV_MEM_RBP_OPEN);
			blat_blist(g->fun_prol, var->name->view); // name
			fun_prol_add(')');
			fun_prol_add(' ');

			// register that is need to put there
			reg = get_regs_of_size(var->lvar_size, mem_counter);

			blat(g->fun_prol, (uc *)reg->name, reg->len);
			fun_prol_add('\n');

			mem_counter++;
		}

		last_offset = arg->offset;
		arg = plist_get(in->os, i);
	}

	// sub rsp instead of one
	// if (g->stack_counter) {
	// 	blat_str_text(STR_ASM_SUB_RSP);
	// 	num_add(g->text, -g->stack_counter);
	// 	text_add('\n');
	// }
	return i;
}

void gen_glob_expr_Асм_Linux_64(struct Gner *g, struct GlobVar *var);
const char SA_LET_8[] = "пусть байт ";
const char SA_LET_16[] = "пусть дбайт ";
const char SA_LET_32[] = "пусть чбайт ";
const char SA_LET_64[] = "пусть вбайт ";

const uint32_t SA_LET_8_LEN = loa(SA_LET_8);
const uint32_t SA_LET_16_LEN = loa(SA_LET_16);
const uint32_t SA_LET_32_LEN = loa(SA_LET_32);
const uint32_t SA_LET_64_LEN = loa(SA_LET_64);

void lay_down_int_Асм_Linux_64(struct Gner *g, struct GlobVar *var) {
	struct Token *num = var->value->tvar;
	enum TypeCode code = var->type->code;

	if (code == TC_INT8 || code == TC_UINT8)
		iprint_prol(SA_LET_8);
	else if (code == TC_INT16 || code == TC_UINT16)
		iprint_prol(SA_LET_16);
	else if (code == TC_INT32 || code == TC_UINT32 || code == TC_ENUM)
		iprint_prol(SA_LET_32);
	else if (code == TC_INT64 || code == TC_UINT64 || code == TC_VOID)
		iprint_prol(SA_LET_64);
	else
		exit(444);

	int_add(g->prol, num->number);
	blat_str_prol(SA_START_COMMENT); // \t;
	hex_int_add(g->prol, num->number);
	prol_add('\n');
}
void lay_down_real_Асм_Linux_64(struct Gner *g, struct GlobVar *var) {
	if (var->type->code == TC_DOUBLE)
		iprint_prol(SA_LET_64);
	else
		iprint_prol(SA_LET_32);

	real_add(g->prol, var->value->tvar->fpn);
	prol_add('\n');
}
void lay_down_str_Асм_Linux_64(struct Gner *g, struct GlobVar *var) {
	iprint_prol(SA_LET_8);
	blat_blist(g->prol, var->value->tvar->view);
	// zero terminator
	prol_add(' ');
	prol_add('0');
	prol_add('\n');
}
void lay_down_gptr_Асм_Linux_64(struct Gner *g, struct GlobVar *var) {
	uc type_size = var->gvar_size;

	if (type_size == QWORD)
		iprint_prol(SA_LET_64);
	else
		// i donno like if like dunno
		iprint_prol(SA_LET_32);

	blat_blist(g->prol, var->value->from->signature);
	prol_add('\n');
}
// void lay_down_arr_Асм_Linux_64(struct Gner *g, struct GlobVar *var) {
// 	uint32_t i;
// 	struct GlobExpr *ge;
// 	struct GlobVar *gvar = malloc(sizeof(struct GlobVar));
// 	gvar->type = var->type;
//
// 	for (i = 0; i < var->value->globs->size; i++) {
// 		ge = plist_get(var->value->globs, i);
//
// 		gen_glob_expr_Асм_Linux_64(g, );
// 	}
// }

void gen_glob_expr_Асм_Linux_64(struct Gner *g, struct GlobVar *var) {
	enum CT_Code code = var->value->code;

	if (code == CT_INT)
		lay_down_int_Асм_Linux_64(g, var);
	else if (code == CT_REAL)
		lay_down_real_Асм_Linux_64(g, var);
	else if (code == CT_STR)
		lay_down_str_Асм_Linux_64(g, var);
	else if (code == CT_GLOBAL_PTR)
		lay_down_gptr_Асм_Linux_64(g, var);
	else if (code == CT_ARR)
		; // lay_down_arr_Асм_Linux_64(g, var);
}
