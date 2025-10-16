#include "prep.h"
#include <stdio.h>

struct PList *parse_macro_args_nodes(struct NodeToken **c, struct Macro *macro);

struct NodeToken *call_macro(struct NodeToken *c, struct Macro *macro) {
	struct NodeToken *fst_at, *lst_at, *macro_call;

	// all of args_nodes nodes need to clone when insert
	// last two values of the list are first args token and last args token
	struct PList *args_nodes = parse_macro_args_nodes(&c, macro);
	// just save args tokens and clear list of them
	fst_at = plist_get(args_nodes, args_nodes->size - 2);
	lst_at = plist_get(args_nodes, args_nodes->size - 1);
	args_nodes->size -= 2;
	// here apply args nodes to args usages
	struct Nodes *body = gen_body(macro->body, macro->args, args_nodes);
	// replace_nodes_inclusive
	macro_call = cut_off_inclusive(fst_at, lst_at);
	c = replace_nodes_inclusive(macro_call, body);

	return c;
}

// ####################################################################
// 						BELOW IS CALL GEN
// ####################################################################

int figure_out_if_its_arg(struct PList *args, struct NodeToken *node) {
	if (!args)
		return -1;

	struct Token *arg;
	uint32_t i;

	foreach_begin(arg, args);
	if (vc(node->token, arg))
		return i;
	foreach_end;

	return -1;
}

// [* Token] args
// [* struct Nodes] args_nodes
struct Nodes *gen_body(struct Nodes *body, struct PList *args,
					   struct PList *args_nodes) {
	body = copy_nodeses(0, body);
	struct Nodes *arg_nodes;
	struct NodeToken *c;
	int arg_index;

	for (c = body->fst; c; c = c->next) {
		arg_index = figure_out_if_its_arg(args, c);
		if (arg_index == -1)
			continue;

		arg_nodes = copy_nodeses(0, plist_get(args_nodes, arg_index));
		c = replace_nodes_inclusive(c, arg_nodes);
	}

	return body;
}

// ####################################################################
// 						BELOW IS CALL PARSE
// ####################################################################

constr PAR_WASNT_CLOSED = "'(' не была закрыта.";
constr CLOSE_PAR = "закрыть '('";
constr PAR_ARR_WASNT_CLOSED = "'[' не была закрыта.";
constr CLOSE_PAR_ARR = "закрыть '['";
constr PAR_CUR_WASNT_CLOSED = "'{' не была закрыта.";
constr CLOSE_PAR_CUR = "закрыть '{'";
constr NOTHING_TO_CLOSE =
	"Встречена закрывающая скобка, но не было открывающей.";
constr CANT_HAVE_EMPTY_ARG_YET = "Аргумент не может быть пустым.";
constr NEED_CLOSE_BY_PAR_R = "Аргумент должен был быть закрыт ')'.";
constr NEED_CLOSE_BY_COMMA = "Аргумент должен был быть закрыт ','.";

struct BList *last_opened_stack = 0;

void exit_par(struct Token *token) {
	if (last_opened_stack->size <= 0)
		eet(token, NOTHING_TO_CLOSE, 0);

	enum TCode last_closed = b_last(last_opened_stack);

	if (last_closed == PAR_L && token->code != PAR_R)
		eet(token, PAR_WASNT_CLOSED, CLOSE_PAR);
	if (last_closed == PAR_C_L && token->code != PAR_C_R)
		eet(token, PAR_ARR_WASNT_CLOSED, CLOSE_PAR_ARR);
	if (last_closed == PAR_T_L && token->code != PAR_T_R)
		eet(token, PAR_CUR_WASNT_CLOSED, CLOSE_PAR_CUR);

	last_opened_stack->size--;
}

enum TCode close_code;
// here all token can be just guaranteed cuz they will be applyed
// while in macro body, so no need to do it twice
struct Nodes *parse_macro_arg_nodes(struct NodeToken **c) {
	struct Nodes *arg_nodes = malloc(sizeof(struct Nodes));
	enum TCode code = (*c)->token->code;

	if (code == close_code)
		eet((*c)->token, CANT_HAVE_EMPTY_ARG_YET, 0);

	if (!last_opened_stack)
		last_opened_stack = new_blist(16);
	else
		last_opened_stack->size = 0;

	arg_nodes->fst = *c;

	for (;; (*c) = take_guaranteed_next(*c)) {
		code = (*c)->token->code;

		if (last_opened_stack->size == 0) {
			if (code == close_code)
				break;
			if (code == COMMA && close_code == PAR_R)
				eet((*c)->token, NEED_CLOSE_BY_PAR_R, 0);
			if (code == PAR_R && close_code == COMMA)
				eet((*c)->token, NEED_CLOSE_BY_COMMA, 0);
		}

		if (code == PAR_L || code == PAR_C_L || code == PAR_T_L)
			blist_add(last_opened_stack, code);
		else if (code == PAR_R || code == PAR_C_R || code == PAR_T_R)
			exit_par((*c)->token);
	}

	arg_nodes->lst = (*c)->prev; // prev cuz c is close_code

	(*c) = take_guaranteed_next(*c); // skip close_code
	return arg_nodes;
}

constr EXPECTED_PAR_R_AFTER_MACRO_ARGS =
	"Ожидалась ')' после аргументов макро без аргументов.";
constr EXPECTED_COMMA_AFTER_MACRO_ARGS =
	"Ожидалась ',' после аргументов макро без аргументов.";

struct PList *parse_macro_args_nodes(struct NodeToken **c,
									 struct Macro *macro) {
	struct PList *args_nodes = new_plist(MAX_ARGS_ON_REGISTERS);
	struct NodeToken *fst_at, *lst_at;
	uint32_t i;

	(*c) = take_guaranteed_next(*c); // here its takes one after macro name
	fst_at = *c;

	// if not parentices then why not just begin args
	if ((*c)->token->code == PAR_L) {
		*c = take_guaranteed_next(*c);
		close_code = PAR_R;
	} else
		close_code = COMMA;

	if (macro->args == 0)
		goto end_macro_call_args;
	if (macro->args->size) {
		if (close_code == PAR_R) {
			// close all args before last with ','
			close_code = COMMA;
			for (i = 0; i < macro->args->size - 1; i++)
				plist_add(args_nodes, parse_macro_arg_nodes(c));
			// for last arg close with ')'
			close_code = PAR_R;
			plist_add(args_nodes, parse_macro_arg_nodes(c));
		} else {
			// close all with ','
			for (i = 0; i < macro->args->size; i++)
				plist_add(args_nodes, parse_macro_arg_nodes(c));
		}
	} else {
	end_macro_call_args:
		if (close_code == PAR_R && ((*c)->token->code != PAR_R))
			eet((*c)->token, EXPECTED_PAR_R_AFTER_MACRO_ARGS, 0);
		if (close_code == COMMA && (*c)->token->code != COMMA)
			eet((*c)->token, EXPECTED_COMMA_AFTER_MACRO_ARGS, 0);
		*c = take_guaranteed_next(*c);
	}

	lst_at = (*c)->prev;

	plist_add(args_nodes, fst_at);
	plist_add(args_nodes, lst_at);

	return args_nodes;
}

// ####################################################################
// 						BELOW IS DELCARATION PARSE
// ####################################################################

struct NodeToken *parse_macro_args(struct NodeToken *c, struct Macro *macro) {
	macro->args = new_plist(4);
	c = take_guaranteed_next(c); // skip '('

	for (; c->token->code != PAR_R; c = take_guaranteed_next(c)) {
		if (c->token->code != ID)
			eet(c->token, EXPECTED_ID_AS_MACRO_ARG, 0);

		plist_add(macro->args, deep_clone_token(c->token, c->token->p));
	}
	// here c should be ')'
	return take_guaranteed_next(c);
}

struct NodeToken *parse_se(struct Prep *pr, struct NodeToken *c) {
	struct Macro *macro = malloc(sizeof(struct Macro));

	// - parse statement
	c = take_guaranteed_next(c);
	if (c->token->code != ID)
		eet(c->token, EXPECTED_ID_AS_MACRO_NAME, 0);
	macro->name = deep_clone_token(c->token, c->token->p);

	c = take_guaranteed_next(c);
	// parse args
	if (c->token->code == SH_L)
		macro->args = 0;
	else if (c->token->code == PAR_L) {
		c = parse_macro_args(c, macro);
		expect(c->token, SH_L);
	} else
		eet(c->token, EXPCEPTED_PAR_L_OR_SH_L, 0);
	// parse block
	macro->body = parse_body(&c);
	// - save statement
	plist_add(pr->macros, macro);

	return c;
}
