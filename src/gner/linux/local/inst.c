#include "../../gner.h"
#include <stdio.h>

void put_vars_on_the_stack(struct Gner *g, struct Inst *in);
void gen_return(Gg, struct LocalExpr *e);

constr CHANGE_VAR_NAME_OR_DELETE_VAR = "изменить имя переменной или удалить ее";
constr CHANGE_LABEL_NAME_OR_DELETE_LABEL = "изменить имя метки или удалить ее";
constr REDEFINING_OF_LOCAL_VAR = "Переопределение локальной переменной.";
constr REDEFINING_OF_LOCAL_LABEL = "Переопределение локальной метки.";
constr CANT_CAST_E_TYPE_TO_RETURN_TYPE =
	"Тип возвращаемого выражения несовместим с возвращаемым типом функции.";
constr EXPRESSION_ISNT_CALCULABLE =
	"Вычисляемое выражение не может возвращать результат.";
constr EXPECTED_EXPR_TO_HAVE_TUPLE = "Ожидалась связка выражений.";
constr WRONG_RETURN_TUPLES_ITEMS =
	"Неверные возвращаемая связка выражений для данной сигнатуры функции.";

void gen_block(Gg, struct PList *os) {
	struct Inst *block_in;
	g->indent_level++;

	for (u32 i = 0; i < os->size; i++) {
		block_in = plist_get(os, i);
		gen_local_linux(g, block_in);
		if (block_in->code == IP_RETURN)
			break;
	}

	g->indent_level--;
}

void gen_local_linux(struct Gner *g, struct Inst *in) {
	struct Token *tok, *name, *str;
	struct BList *string;
	struct Loop *loop;
	uint32_t i = 0;

	switch (in->code) {
	case IP_ASM:
		// ### os explanation:
		//   _ - assembly string token

		str = plist_get(in->os, 0);
		blat_ft(str->str);
		break;
	case IP_LET:
		// ### os explanation
		// ... - Arg's

		put_vars_on_the_stack(g, in);
		break;
	case IP_DECLARE_LABEL:
		// ### os explanation
		//   _ - label name token

		name = plist_get(in->os, 0);

		for (; i < g->local_labels->size; i++) {
			tok = plist_get(g->local_labels, i);
			if (vc(tok, name))
				eet(name, REDEFINING_OF_LOCAL_LABEL,
					CHANGE_LABEL_NAME_OR_DELETE_LABEL);
		}
		plist_add(g->local_labels, name);

		g->indent_level--;
		{
			indent_line(g, g->fun_text);
			blat_ft(g->current_function->signature);
			blat_ft(name->view), ft_add(':'), ft_add('\n');
		}
		g->indent_level++;
		break;
	case IP_GOTO:
		// ### os explanation
		//   _ - label name token

		name = plist_get(in->os, 0);

		op_(JMP);
		blat_ft(g->current_function->signature);
		blat_ft(name->view);
		ft_add('\n');
		break;
	case IP_LOOP:
		// ### os explanation
		// ... - local instructions
		//   _ - struct Loop *

		loop = p_last(in->os);
		in->os->size--; // remove loop from os

		string = loop->cont ? loop->cont : take_label(LC_LOOP);
		add_label(string);

		gen_block(g, in->os);
		jmp_(string);

		blist_clear_free(string); // free cont
		if (loop->brek) {
			add_label(loop->brek);
			blist_clear_free(string); // free brek
		}
		free(loop);
		break;
	case IP_LOCAL_EXPRESSION:
		gen_local_expr_inst_linux(g, in);
		break;
	case IP_BREAK:
	case IP_CONTINUE:
		// ### os explanation
		//   _ - label to jmp to

		string = plist_get(in->os, 0);
		jmp_(string);
		blist_clear_free(string);
		break;
	case IP_RETURN:
		// ### os explanation
		//   _ - return LocalExpr *

		gen_return(g, plist_get(in->os, 0));
		break;
	case IP_NONE:
	default:
		eei(in, "эээ", 0);
	}
}

void put_vars_on_the_stack(struct Gner *g, struct Inst *in) {
	uint32_t i, j, vars;
	struct Arg *arg;
	struct LocalVar *var, *tmp_var;
	long last_offset = -1;

	for (i = 0; i < in->os->size; i++) {
		arg = plist_get(in->os, i);
		if (arg->offset != last_offset)
			g->stack_counter -= arg->arg_size;

		for (j = 0; j < arg->names->size; j++) {
			var =
				new_local_var(plist_get(arg->names, j), arg, g->stack_counter);

			for (vars = 0; vars < g->local_vars->size; vars++) {
				tmp_var = plist_get(g->local_vars, vars);

				if (vc(tmp_var->name, var->name))
					eet(var->name, REDEFINING_OF_LOCAL_VAR,
						CHANGE_VAR_NAME_OR_DELETE_VAR);
			}

			plist_add(g->local_vars, var);

			iprint_after_stack_frame(SA_EQU);		 // вот
			blat_after_stack_frame(var->name->view); // name
			after_stack_frame_add(' ');
			add_int_with_hex_comm(after_stack_frame, g->stack_counter);
		}

		last_offset = arg->offset;
	}
}

int can_cast_type(struct LocalExpr *e, struct TypeExpr *cast_type) {
	struct TypeExpr *from_type = e->type;
	return is_ptr_type(cast_type) && is_ptr_type(from_type)
			   ? are_types_equal(cast_type, from_type)
		   : is_ptr_type(cast_type) && is_le_num(e, 0)
			   ? 1
			   : is_num_type(cast_type) && is_num_type(from_type);
}

struct Reg *try_return_to(Gg, struct TypeExpr *return_type, struct LocalExpr *e,
						  struct RegisterFamily *to_rf) {
	print_local_expr_to_file(e);

	struct Reg *r;
	if (!can_cast_type(e, return_type))
		eet(e->tvar, CANT_CAST_E_TYPE_TO_RETURN_TYPE, 0);
	int return_type_size = unsafe_size_of_type(return_type);

	if (lcep(INT) && is_real_type(return_type)) {
		e->tvar->real = e->tvar->num;
		goto real;
	} else if (lcep(REAL)) {
	real:
		gen_tuple_of(g, e);
		r = try_borrow_reg(e->tvar, g, return_type_size);
		op_reg_(MOV, r);
		if (is_int_type(return_type)) {
			e->tvar->num = e->tvar->real;
			add_int_with_hex_comm(fun_text, e->tvar->num);
		} else {
			real_add_enter(fun_text, e->tvar->real);
		}
	} else {
		r = gen_to_reg(g, e, return_type_size);
	}

	if (is_xmm(r)) {
		if (is_int_type(return_type)) {
			r = cvt_from_xmm(g, e, r);
		} else {
			struct Reg *r2 = try_borrow_reg(e->tvar, g, return_type_size);
			op_reg_reg(MOV_XMM, r2, r);
			free_reg(r);
			r = r2;
		}
	}
	r = get_reg_to_size(g, r, return_type_size);
	get_reg_to_rf(e->tvar, g, r, to_rf);
	return r;
}

void try_return_tuple(Gg, struct TypeExpr *return_type, struct LocalExpr *e) {
	if (!e->tuple)
		eet(e->tvar, EXPECTED_EXPR_TO_HAVE_TUPLE, 0);
	if (e->tuple->size + 1 != tup_itms(return_type)->size)
		eet(e->tvar, WRONG_RETURN_TUPLES_ITEMS, 0);

	print_local_expr_to_file(e);

	struct PList *return_tuple_types = tup_itms(return_type);
	struct PList *return_tuple = e->tuple;
	plist_add(return_tuple, e);
	e->tuple = 0;

	struct PList *regs = new_plist(return_tuple->size);
	u32 i;
	struct Reg *r;
	struct RegisterFamily *rf;
	struct LocalExpr *return_item;
	struct TypeExpr *return_item_type;

	for (i = 0; i < return_tuple->size; i++) {
		rf = as_rfs(g->cpu)[return_tuple_regs_indeces[i]];
		return_item_type = plist_get(return_tuple_types, i);
		return_item = plist_get(return_tuple, i);

		r = try_return_to(g, return_item_type, return_item, rf);
		plist_add(regs, r);
	}
	for (i = 0; i < regs->size; i++) {
		r = plist_get(regs, i);
		free_register(r);
	}
	plist_free(regs);
}

uc function_body_return = 0;
void gen_return(Gg, struct LocalExpr *e) {
	struct TypeExpr *return_type = find_return_type(g->current_function->type);

	if (!e) {
		if (return_type->code != TC_VOID)
			eet(e->tvar, CANT_CAST_E_TYPE_TO_RETURN_TYPE, 0);
		goto ret_or_jmp;
	}

	define_type_and_copy_flags_to_e(e);
	if (!e->type)
		eet(e->tvar, EXPRESSION_ISNT_CALCULABLE, 0);
	opt_bin_constant_folding(e);

	return_type->code == TC_TUPLE
		? try_return_tuple(g, return_type, e)
		: free_register(try_return_to(g, return_type, e, g->cpu->a));

ret_or_jmp:
	if (function_body_return) {
		if (g->label_to_ret)
			add_label(g->label_to_ret);
		write_flags_and_end_stack_frame(g);
	} else {
		if (!g->label_to_ret)
			g->label_to_ret = take_label(LC_ELSE);
		jmp_(g->label_to_ret);
	}
}
