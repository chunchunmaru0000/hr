#include "../gner.h"
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
const char SA_ZERO_TERMINATOR[] = " 0\n";

const uint32_t SA_EQU_LEN = loa(SA_EQU);
const uint32_t SA_PUSH_RBP_LEN = loa(SA_PUSH_RBP);
const uint32_t SA_MOV_RBP_RSP_LEN = loa(SA_MOV_RBP_RSP);
const uint32_t SA_MOV_MEM_RBP_OPEN_LEN = loa(SA_MOV_MEM_RBP_OPEN);
const uint32_t SA_START_COMMENT_LEN = loa(SA_START_COMMENT);
const uint32_t SA_SUB_RSP_LEN = loa(SA_SUB_RSP);
const uint32_t SA_LEAVE_LEN = loa(SA_LEAVE);
const uint32_t SA_RET_LEN = loa(SA_RET);
const uint32_t SA_ZERO_TERMINATOR_LEN = loa(SA_ZERO_TERMINATOR);

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
		g->current_inst = in;
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
				bprol_add('\t');
				int_add(g->bprol, (long)defn->value);
				bprol_add('\n');
			}

			bprol_add('\n');
			break;
		case IP_DECLARE_STRUCT:
			// ### os explanation:
			//   _ - name
			//   _ - size
			//   _ - mems size, mems are not args
			// ... - fields that are Arg's

			tok = plist_get(in->os, 0);

			for (j = DCLR_STRUCT_ARGS; j < in->os->size; j++)
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

			g->tmp_blist = gen_glob_expr_Асм_Linux_64(g, global_var->value);
			copy_to_fst_and_clear_snd(g->prol, g->tmp_blist);

			g->indent_level--;
			break;
		case IP_NONE:
		default:
			eei(in, "ээЭэ", 0);
		}
	}
end_gen_Асм_text_loop:;

	aprol_add('\n');
	iprint_aprol(SA_SEGMENT_READ_EXECUTE);
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

const char SA_LET_8[] = "пусть байт ";
const char SA_LET_16[] = "пусть дбайт ";
const char SA_LET_32[] = "пусть чбайт ";
const char SA_LET_64[] = "пусть вбайт ";
const char SA_REZERV_ZERO[] = "запас 0 ";

const uint32_t SA_LET_8_LEN = loa(SA_LET_8);
const uint32_t SA_LET_16_LEN = loa(SA_LET_16);
const uint32_t SA_LET_32_LEN = loa(SA_LET_32);
const uint32_t SA_LET_64_LEN = loa(SA_LET_64);
const uint32_t SA_REZERV_ZERO_LEN = loa(SA_REZERV_ZERO);

struct BList *clear_current_inst_value_labels_to(struct Gner *g,
												 struct BList *label) {
	struct GlobVar *this_e_var;
	struct BList *freed = 0;
	uint32_t i;

	// clear current vars value_label if it was set during compilation
	for (i = 0; i < g->current_inst->os->size; i++) {
		this_e_var = plist_get(g->current_inst->os, i);

		if (this_e_var->value_label) {

			if (this_e_var->value_label && freed != this_e_var->value_label) {
				freed = this_e_var->value_label;
				blist_clear_free(this_e_var->value_label);
			}
			this_e_var->value_label = label;
		}
	}
}

struct BList *lay_down_int_Асм_Linux_64(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(32);
	enum TypeCode code = e->type->code;

	if (code == TC_INT8 || code == TC_UINT8)
		iprint_gen(SA_LET_8);
	else if (code == TC_INT16 || code == TC_UINT16)
		iprint_gen(SA_LET_16);
	else if (code == TC_INT32 || code == TC_UINT32 || code == TC_ENUM)
		iprint_gen(SA_LET_32);
	else if (code == TC_INT64 || code == TC_UINT64 || code == TC_VOID)
		iprint_gen(SA_LET_64);
	else {
		printf("#ERR_INFO. e->type->code was %d\n", code);
		exit(223);
	}

	int_add(generated, e->tvar->num);
	print_gen(SA_START_COMMENT); // \t;
	hex_int_add(generated, e->tvar->num);
	gen_add('\n');

	return generated;
}
struct BList *lay_down_real_Асм_Linux_64(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(32);

	if (e->type->code == TC_DOUBLE)
		iprint_gen(SA_LET_64);
	else
		iprint_gen(SA_LET_32);

	real_add(generated, e->tvar->real);
	gen_add('\n');

	return generated;
}
struct BList *lay_down_str_Асм_Linux_64(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(32);

	iprint_gen(SA_LET_8);
	blat_blist(generated, e->tvar->view);
	print_gen(SA_ZERO_TERMINATOR);

	return generated;
}
struct BList *lay_down_gptr_Асм_Linux_64(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(32);

	iprint_gen(SA_LET_64);
	blat_blist(generated, e->from->signature);
	gen_add('\n');

	return generated;
}

uc need_to_gen_ptr = 1;

struct BList *lay_down_obj_Асм_Linux_64(struct Gner *g, struct GlobExpr *e) {

	struct BList *generated = new_blist(64), *tmp_gen;
	struct PList *labels = new_plist(2);
	struct GlobExpr *glob;
	uint32_t i;

	for (i = 0; i < e->globs->size; i++) {
		glob = plist_get(e->globs, i);

		if (glob->code == CT_ARR_PTR || glob->code == CT_STRUCT_PTR) {
			need_to_gen_ptr = 0;

			g->indent_level += 2;

			// save generared code to labels
			tmp_gen = gen_glob_expr_Асм_Linux_64(g, glob);
			plist_add(labels, tmp_gen);

			g->indent_level -= 2;

			// save label
			tmp_gen = take_label(g, LC_PTR);
			plist_add(labels, tmp_gen);

			// lay label
			iprint_gen(SA_LET_64);
			blat_blist(generated, tmp_gen);
			gen_add('\n');

			need_to_gen_ptr = 1;
		} else {
			// just gen
			tmp_gen = gen_glob_expr_Асм_Linux_64(g, glob);
			copy_to_fst_and_clear_snd(generated, tmp_gen);
		}
	}

	g->indent_level += 1;

	for (i = 0; i < labels->size; i += 2) {
		indent_line(g, generated);
		// decalre label
		tmp_gen = plist_get(labels, i + 1); // label
		copy_to_fst_and_clear_snd(generated, tmp_gen);
		print_gen(SA_LABEL_END); // :

		// lay generared code
		tmp_gen = plist_get(labels, i); // code
		copy_to_fst_and_clear_snd(generated, tmp_gen);
	}

	g->indent_level -= 1;

	plist_free(labels);
	clear_current_inst_value_labels_to(g, 0);
	return generated;
}
struct BList *lay_down_obj_ptr_Асм_Linux_64(struct Gner *g,
											struct GlobExpr *e) {

	struct BList *generated = new_blist(64), *ptr, *tmp_gen;
	uc was_need_to_gen_ptr = need_to_gen_ptr;

	if (was_need_to_gen_ptr) {
		ptr = take_label(g, LC_PTR);
		// declare label
		iprint_gen(SA_LET_64);
		blat_blist(generated, ptr);
		gen_add('\n');

		// lay label
		blat_blist(generated, ptr);
		print_gen(SA_LABEL_END); // :
	}

	tmp_gen = lay_down_obj_Асм_Linux_64(g, e);
	copy_to_fst_and_clear_snd(generated, tmp_gen);

	if (was_need_to_gen_ptr) {
		clear_current_inst_value_labels_to(g, ptr);
		// blist_clear_free(ptr); no need
	}

	need_to_gen_ptr = was_need_to_gen_ptr;
	return generated;
}
struct BList *lay_down_str_ptr_Асм_Linux_64(struct Gner *g,
											struct GlobExpr *e) {
	struct BList *generated = new_blist(64);
	iprint_gen(SA_LET_64);

	if (!e->from) {
	add_value_ptr_to_this_e_var:

		struct BList *ptr = take_label(g, LC_PTR);
		struct GlobVar *this_e_var;

		for (uint32_t j = 0; j < g->current_inst->os->size; j++) {
			this_e_var = plist_get(g->current_inst->os, j);
			this_e_var->value_label = ptr;
		}

		blat_blist(generated, ptr);
		gen_add('\n');

		blat_blist(g->aprol, ptr);
		print_aprol(SA_LABEL_END); // :

		aprol_add('\t');
		print_aprol(SA_LET_8);
		blat_blist(g->aprol, e->tvar->view);
		print_aprol(SA_ZERO_TERMINATOR);

	} else if (e->from) {
		if (e->from->value_label) {
			blat_blist(generated, e->from->value_label);
			gen_add('\n');
		} else {
			// not sure about this but
			// even if its possible it seems to be the case
			goto add_value_ptr_to_this_e_var;
		}
	}

	return generated;
}

struct BList *lay_down_zero(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated = new_blist(64);

	// e->tvar->num of CT_ZERO is arg_size
	struct BList *times = int_to_str(e->tvar->num);

	iprint_gen(SA_LET_8);
	print_gen(SA_REZERV_ZERO);
	copy_to_fst_and_clear_snd(generated, times);
	gen_add('\n');

	return generated;
}

struct BList *gen_glob_expr_Асм_Linux_64(struct Gner *g, struct GlobExpr *e) {
	struct BList *generated;
	enum CT_Code code = e->code;

	if (code != CT_ARR && e->type->code == TC_ARR) {
		e->type->code = arr_type(e->type)->code;
		generated = gen_glob_expr_Асм_Linux_64(g, e);
		e->type->code = TC_ARR;
	} else if (code == CT_INT)
		generated = lay_down_int_Асм_Linux_64(g, e);
	else if (code == CT_REAL)
		generated = lay_down_real_Асм_Linux_64(g, e);
	else if (code == CT_STR)
		generated = lay_down_str_Асм_Linux_64(g, e);
	else if (code == CT_GLOBAL_PTR)
		generated = lay_down_gptr_Асм_Linux_64(g, e);
	else if (code == CT_STR_PTR)
		generated = lay_down_str_ptr_Асм_Linux_64(g, e);
	else if (code == CT_ARR_PTR || code == CT_STRUCT_PTR)
		generated = lay_down_obj_ptr_Асм_Linux_64(g, e);
	else if (code == CT_ARR || code == CT_STRUCT)
		generated = lay_down_obj_Асм_Linux_64(g, e);
	else if (code == CT_ZERO)
		generated = lay_down_zero(g, e);

	return generated;
}
