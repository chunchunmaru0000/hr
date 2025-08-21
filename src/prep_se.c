#include "prep.h"

const char *const EXPECTED_PAR_L_AFTER_MACRO_NAME =
	"Ожидалась '(' после имени макро.";

struct PList *

struct NodeToken *call_macro(struct Prep *pr, struct NodeToken *c,
							 struct Macro *macro) {
	struct NodeToken *fst = c;

	c = take_applyed_next(pr, c);
	if (c->token->code != PAR_L)
		eet(c->token, EXPECTED_PAR_L_AFTER_MACRO_NAME, 0);

	
}

struct NodeToken *parse_macro_args(struct Prep *pr, struct NodeToken *c,
								   struct Macro *macro) {
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

struct NodeToken *parse_macro_block(struct Prep *pr, struct NodeToken *c,
									struct Macro *macro) {
	struct NodeToken *clone, *clone_prev;
	macro->body = malloc(sizeof(struct NodeToken));

	c = take_guaranteed_next(c); // skip '(#'

	// here need to copy tree cuz it will be freed from final tokens
	clone = clone_node_token(c); // first is different
	figure_out_if_its_arg(macro, clone);
	clone->prev = 0;
	macro->body->fst = clone;

	clone_prev = clone;
	c = take_applyed_next(pr, c);
	for (; c->token->code != SH_R; c = take_applyed_next(pr, c)) {
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
		c = parse_macro_args(pr, c, macro);
		expect(c->token, SH_L);
	} else
		eet(c->token, EXPCEPTED_PAR_L_OR_SH_L, 0);
	// parse block
	c = parse_macro_block(pr, c, macro);

	// - save statement
	plist_add(pr->macros, macro);

	return c;
}
