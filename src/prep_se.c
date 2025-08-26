#include "prep.h"
#include <stdio.h>

const char *const PAR_WASNT_CLOSED = "'(' не была закрыта.";
const char *const CLOSE_PAR = "закрыть '('";
const char *const PAR_ARR_WASNT_CLOSED = "'[' не была закрыта.";
const char *const CLOSE_PAR_ARR = "закрыть '['";
const char *const PAR_CUR_WASNT_CLOSED = "'{' не была закрыта.";
const char *const CLOSE_PAR_CUR = "закрыть '{'";
const char *const NOTHING_TO_CLOSE =
	"Встречена закрывающая скобка, но не было открывающей.";

struct BList *last_opened_stack = 0;

void exit_par(struct Token *token) {
	enum TCode last_closed = b_last(last_opened_stack);

	if (last_opened_stack->size <= 0)
		eet(token, NOTHING_TO_CLOSE, 0);
	if (last_closed == PAR_L && token->code != PAR_R)
		eet(token, PAR_WASNT_CLOSED, CLOSE_PAR);
	if (last_closed == PAR_C_L && token->code != PAR_C_R)
		eet(token, PAR_ARR_WASNT_CLOSED, CLOSE_PAR_ARR);
	if (last_closed == PAR_T_L && token->code != PAR_T_R)
		eet(token, PAR_CUR_WASNT_CLOSED, CLOSE_PAR_CUR);

	last_opened_stack->size--;
}

// here all token can be just guaranteed cuz they will be applyed
// while in macro body, so no need to do it twice
struct Nodes *parse_macro_arg_nodes(struct NodeToken **c) {
	struct Nodes *arg_nodes = malloc(sizeof(struct Nodes));
	enum TCode code;

	arg_nodes->fst = *c;

	(*c) = take_guaranteed_next(*c);
	for (;; (*c) = take_guaranteed_next(*c)) {
		code = (*c)->token->code;

		if ((code == PAR_R || code == COMMA) && last_opened_stack->size == 0)
			break;

		if (code == PAR_L || code == PAR_C_L || code == PAR_T_L)
			blist_add(last_opened_stack, code);
		else if (code == PAR_R || code == PAR_C_R || code == PAR_T_R)
			exit_par((*c)->token);
	}

	arg_nodes->lst = (*c)->prev; // prev cuz c is or PAR_CLOSE or COMMA
	return arg_nodes;
}

const char *const EXPECTED_PAR_R_OR_COMMA_AFTER_MACRO_ARGS =
	"Ожидалась ')' или ',' после аргументов макро.";

struct PList *parse_macro_args_nodes(struct NodeToken *c, struct Macro *macro) {
	struct PList *args_nodes = new_plist(macro->args->size);
	uint32_t i;

	printf("asdf\n");

	c = take_guaranteed_next(c); // here its takes one after macro name

	// if not parentices then why not just begin args
	if (c->token->code == PAR_L)
		c = take_guaranteed_next(c);

	for (i = 0; i < macro->args->size; i++)
		plist_add(args_nodes, parse_macro_arg_nodes(&c));

	if (c->token->code != PAR_R && c->token->code != COMMA)
		eet(c->token, EXPECTED_PAR_R_OR_COMMA_AFTER_MACRO_ARGS, 0);

	return args_nodes;
}

struct NodeToken *call_macro(struct NodeToken *c, struct Macro *macro) {
	struct NodeToken *fst_to_cut = c, *lst_to_cut, *head;
	// all of args_nodes nodes need to clone when insert
	struct PList *args_nodes = parse_macro_args_nodes(c, macro);

	// here apply args nodes to args usages

	if (fst_to_cut->prev)
		fst_to_cut->prev->next = head;
	head->prev = fst_to_cut;

	return head;
}

// ####################################################################
// 						BELOW IS DELCARATION PARSE
// ####################################################################

struct NodeToken *parse_macro_args(struct NodeToken *c, struct Macro *macro) {
	struct MacroArg *arg;

	macro->args = new_plist(4);
	c = take_guaranteed_next(c); // skip '('

	for (; c->token->code != PAR_R; c = take_guaranteed_next(c)) {
		if (c->token->code != ID)
			eet(c->token, EXPECTED_ID_AS_MACRO_ARG, 0);

		arg = malloc(sizeof(struct MacroArg));
		arg->name = c->token;
		arg->usages = new_plist(2);

		plist_add(macro->args, arg);
	}
	// here c should be ')'
	return take_guaranteed_next(c);
}

void figure_out_if_its_arg(struct Macro *macro, struct NodeToken *node) {
	if (macro->args == 0)
		return;

	struct MacroArg *arg;
	uint32_t i;

	foreach_begin(arg, macro->args);
	if (vc(node->token, arg->name))
		plist_add(arg->usages, node);
	foreach_end;
}

// while in parse need to use take_guaranteed_next cuz some of the macros
// that cant be applyed yet will be anyway parsable in take_applyed_next
// when it will be actually called
struct NodeToken *parse_macro_block(struct NodeToken *c, struct Macro *macro) {
	struct NodeToken *clone, *clone_prev;
	macro->body = malloc(sizeof(struct NodeToken));

	c = take_guaranteed_next(c); // skip '(#'

	// here need to copy tree cuz it will be freed from final tokens
	clone = clone_node_token(c); // first is different
	figure_out_if_its_arg(macro, clone);
	clone->prev = 0;
	macro->body->fst = clone;

	clone_prev = clone;
	c = take_guaranteed_next(c);
	for (; c->token->code != SH_R; c = take_guaranteed_next(c)) {
		clone = clone_node_token(c);
		figure_out_if_its_arg(macro, clone);
		clone->prev = clone_prev;
		clone_prev->next = clone;

		clone_prev = clone;
	}
	clone->next = 0;
	macro->body->lst = clone;

	return c; // need to return '#)' cuz it will be last
}

struct NodeToken *parse_se(struct Prep *pr, struct NodeToken *c) {
	struct Macro *macro = malloc(sizeof(struct Macro));

	// - parse statement
	c = take_guaranteed_next(c);
	if (c->token->code != ID)
		eet(c->token, EXPECTED_ID_AS_MACRO_NAME, 0);
	macro->name = c->token;

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
	c = parse_macro_block(c, macro);

	// - save statement
	plist_add(pr->macros, macro);

	return c;
}
