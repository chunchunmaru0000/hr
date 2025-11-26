#include "../../gner.h"
#include <stdio.h>

uint32_t put_args_on_the_stack(struct Gner *g, struct Inst *in);
void declare_struct_arg(struct Gner *g, struct Token *strct, struct Arg *arg);
void write_flags_and_end_stack_frame(Gg);

struct Register {
	const char *const name;
	unsigned char len;
	enum RegCode reg_code;
	int size;
};

const struct Register argument_regs_BYTE[7] = {
	{"си", 4, R_SI, WORD},	  {"ди", 4, R_DI, WORD},
	{"б8", 3, R_R8B, BYTE},	  {"б9", 3, R_R9B, BYTE},
	{"б10", 4, R_R10B, BYTE}, {"б11", 4, R_R11B, BYTE},
	{"б12", 4, R_R12B, BYTE},
};
const struct Register argument_regs_WORD[7] = {
	{"си", 4, R_SI, WORD},	  {"ди", 4, R_DI, WORD},
	{"д8", 3, R_R8W, WORD},	  {"д9", 3, R_R9W, WORD},
	{"д10", 4, R_R10W, WORD}, {"д11", 4, R_R11W, WORD},
	{"д12", 4, R_R12W, WORD},
};
const struct Register argument_regs_DWORD[7] = {
	{"еси", 6, R_ESI, DWORD},  {"еди", 6, R_EDI, DWORD},
	{"е8", 3, R_R8D, DWORD},   {"е9", 3, R_R9D, DWORD},
	{"е10", 4, R_R10D, DWORD}, {"е11", 4, R_R11D, DWORD},
	{"е12", 4, R_R12D, DWORD},
};
const struct Register argument_regs_QWORD[7] = {
	{"рси", 6, R_RSI, QWORD}, {"рди", 6, R_RDI, QWORD},
	{"р8", 3, R_R8, QWORD},	  {"р9", 3, R_R9, QWORD},
	{"р10", 4, R_R10, QWORD}, {"р11", 4, R_R11, QWORD},
	{"р12", 4, R_R12, QWORD},
};

void gen_linux_text(struct Gner *g) {
	uint32_t i, j, local_i;
	struct Inst *in;
	struct Token *tok, *tok2;
	struct GlobVar *global_var;
	struct Enum *enum_obj;

	g->flags->is_data_segment_used = 0;

	for (i = 0; i < g->is->size; i++) {
		in = plist_get(g->is, i);
		g->current_inst = in;
		g->pos = i;

		switch (in->code) {
		case IP_EOI:
			goto end_gen_text_loop;
		case IP_ASM:
			// ### os explanation:
			//   _ - assembly string token

			tok = plist_get(in->os, 0);
			blat_text(tok->str);
			break;
		case IP_DECLARE_ENUM:
			// ### os explanation:
			//   _ - struct Enum *enum_obj

			enum_obj = plist_get(in->os, 0);
			foreach_by(j, tok, enum_obj->items) {
				iprint_bprol(SA_EQU);				   // вот
				blat_bprol(enum_obj->enum_name->view); // enum name
				bprol_add('.');						   // .
				blat_bprol(tok->view);				   // item name
				bprol_add('\t');					   // \t
				add_int_with_hex_comm(bprol, tok->num);
			}
			foreach_end;
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
			reset_flags(g);
			// begin stack frame
			global_var = plist_get(in->os, 0);

			blat_stack_frame(global_var->signature); // fun label
			blat_str_stack_frame(SA_LABEL_END);		 // :
			g->indent_level++;
			iprint_stack_frame(SA_PUSH_RBP);
			iprint_stack_frame(SA_MOV_RBP_RSP);

			local_i = put_args_on_the_stack(g, in);
			// function body
			for (; local_i < in->os->size; local_i++)
				gen_local_linux(g, plist_get(in->os, local_i));
			// free stack in return statement
			write_flags_and_end_stack_frame(g);

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

				blat_prol(global_var->signature);
				blat_str_prol(SA_LABEL_END); // :
			}

			g->indent_level++;

			g->tmp_blist = gen_glob_expr_linux(g, global_var->value);
			copy_to_fst_and_clear_snd(g->prol, g->tmp_blist);

			g->indent_level--;
			break;
		case IP_NONE:
		default:
			eei(in, "ээЭэ", 0);
		}
	}
end_gen_text_loop:;

	aprol_add('\n');
	iprint_aprol(SA_SEGMENT_READ_EXECUTE);
}

void write_flags_and_end_stack_frame(Gg) {
	struct Fggs *f = g->flags;

	if (f->is_stack_used || f->is_r15_used || f->is_r14_used ||
		f->is_r13_used) {
		iprint_stack_frame(SA_SUB_RSP);
		add_int_with_hex_comm(stack_frame, -g->stack_counter);

		if (f->is_r15_used) {
			iprint_stack_frame(SA_PUSH_R15);
			iprint_ft(SA_POP_R15);
		}
		if (f->is_r14_used) {
			iprint_stack_frame(SA_PUSH_R14);
			iprint_ft(SA_POP_R14);
		}
		if (f->is_r13_used) {
			iprint_stack_frame(SA_PUSH_R13);
			iprint_ft(SA_POP_R13);
		}

		iprint_ft(SA_LEAVE);
	} else
		iprint_ft(SA_POP_RBP);

	iprint_ft(SA_RET);
	ft_add('\n');
}

void declare_struct_arg(struct Gner *g, struct Token *strct, struct Arg *arg) {
	struct Token *name;

	for (uint32_t i = 0; i < arg->names->size; i++) {
		iprint_bprol(SA_EQU);

		blat_bprol(strct->view);
		name = plist_get(arg->names, i);
		bprol_add('.');
		blat_bprol(name->view);

		bprol_add('\t');
		add_int_with_hex_comm(bprol, arg->offset);
	}
}

// фц взять_регстры_размера(размер:ч32) *[лик Регистр 7]
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

uint32_t put_args_on_the_stack(struct Gner *g, struct Inst *in) {
	// fun in->os are: fun variable, args..., 0 term, instructions...
	uint32_t i, j, mem_counter = 0;
	struct Arg *arg = plist_get(in->os, 1);
	struct LocalVar *var;
	const struct Register *reg;
	long last_offset = -1;

	for (i = 2; arg; i++) {
		if (arg->offset != last_offset)
			// it wastes 2 bytes on stack if first two args are of BYTE size
			g->stack_counter -= mem_counter > 2			? arg->arg_size
								: arg->arg_size == BYTE ? WORD
														: arg->arg_size;

		for (j = 0; j < arg->names->size; j++) {
			var =
				new_local_var(plist_get(arg->names, j), arg, g->stack_counter);
			plist_add(g->local_vars, var);

			iprint_after_stack_frame(SA_EQU);		 // вот
			blat_after_stack_frame(var->name->view); // name
			after_stack_frame_add(' ');
			add_int_with_hex_comm(after_stack_frame, g->stack_counter);
		}

		if (arg->offset != last_offset) {
			// быть (рсп - g->tmp_blist) register
			iprint_after_stack_frame(SA_MOV_MEM_RBP_OPEN);
			blat_after_stack_frame(var->name->view); // name
			after_stack_frame_add(')');
			after_stack_frame_add(' ');

			// register that is need to put there
			reg = get_regs_of_size(var->lvar_size, mem_counter);

			blat(g->after_stack_frame, (uc *)reg->name, reg->len);
			after_stack_frame_add('\n');

			mem_counter++;
		}

		last_offset = arg->offset;
		arg = plist_get(in->os, i);
	}

	return i;
}
