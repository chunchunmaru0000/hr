#include "prep.h"
#include <stdio.h>

// ####################################################################
// 						BELOW IS SENTENCE CALL GEN
// ####################################################################

// ####################################################################
// 						BELOW IS SENTENCE PARSE
// ####################################################################

const char *const EXPECTED_SENT_START_SH_L =
	"Ожидались скобки '(#' для начала объявления бук.";
const char *const EXPECTED_SENT_BODY_SH_L =
	"Ожидались скобки '(#' для начала объявления тела бук.";
const char *const EXPECTED_SHARP_AS_CLOSING_ARG =
	"Ожидался символ '#' для закрытия аргумета бук, например: слово #арг# "
	"слово.";
/*

#буки (#
	вот #переменная# будет#|станет вообще типа #тип# и значением #значение#
#) (#
		пусть переменная: тип = значение
#)

*/
struct SentenceWord *new_sentence_word(struct Token *t,
									   struct SentenceWord * or) {
	struct SentenceWord *sentence_word = malloc(sizeof(struct SentenceWord));
	sentence_word->word = t;
	sentence_word->or_word = or ;
	return sentence_word;
}

struct SentenceWord *parse_word_or_word(struct PList *words,
										struct NodeToken **c,
										struct NodeToken **n) {
	struct SentenceWord *sentence_word;
	sentence_word =
		new_sentence_word(deep_clone_token((*c)->token, (*c)->token->p), 0);

	if ((*n)->token->code == SH_OR) {
		*c = take_guaranteed_next(*n);
		(*n) = take_guaranteed_next(*c);
		sentence_word->or_word = parse_word_or_word(words, c, n);
	}
	return sentence_word;
}

int until_next_arg(struct PList *words, struct NodeToken **c) {
	struct NodeToken *n;
	enum TCode code;

	*c = take_guaranteed_next(*c);
	loop {
		n = take_guaranteed_next(*c);
		code = (*c)->token->code;

		if (code == SH_R)
			return 0;
		if (code == SHARP)
			return 1;

		plist_add(words, parse_word_or_word(words, c, &n));
		*c = n;
	}
}

void parse_sent_args_and_words(struct Prep *pr, struct Sentence *sent,
							   struct NodeToken **name) {
	struct PList *args = new_plist(8);
	struct PList *words = new_plist(16);
	struct NodeToken *c = next_applyed(pr, *name);

	if (c->token->code != SH_L)
		eet(c->token, EXPECTED_SENT_START_SH_L, 0);

	while (until_next_arg(words, &c)) {
		c = take_guaranteed_next(c);
		if (c->token->code != ID)
			eet(c->token, EXPECTED_ID_AS_MACRO_NAME, 0);

		plist_add(args, deep_clone_node(c));

		c = take_guaranteed_next(c);
		if (c->token->code != SHARP)
			eet(c->token, EXPECTED_SHARP_AS_CLOSING_ARG, 0);
	}

	*name = take_guaranteed_next(c); // skip '#)' and ret with'(#'
	sent->args = args;
	sent->words = words;
}

struct Nodes *parse_body(struct NodeToken **start) {
	struct Nodes *body = malloc(sizeof(struct Nodes));
	struct NodeToken *c = *start, *clone, *clone_prev;

	if (c->token->code != SH_L)
		eet(c->token, EXPECTED_SENT_BODY_SH_L, 0);
	c = take_guaranteed_next(c); // skip '(#'

	// here need to copy tree cuz it will be freed from final tokens
	clone = clone_node_token(c); // first is different
	clone->prev = 0;
	body->fst = clone;

	clone_prev = clone;
	c = take_guaranteed_next(c);
	for (; c->token->code != SH_R; c = take_guaranteed_next(c)) {
		clone = clone_node_token(c);
		clone->prev = clone_prev;
		clone_prev->next = clone;

		clone_prev = clone;
	}
	clone->next = 0;
	body->lst = clone;

	*start = c; // need to return '#)' in start
	return body;
}

void debug_sent(struct Sentence *sent);

struct NodeToken *parse_sent(struct Prep *pr, struct NodeToken *name) {
	struct NodeToken *c = name;
	struct Sentence *sent = malloc(sizeof(struct Sentence));
	plist_add(pr->sentences, sent);

	parse_sent_args_and_words(pr, sent, &c);
	sent->body = parse_body(&c);

	// debug_sent(sent);
	return c;
}

void debug_sent(struct Sentence *sent) {
	uint32_t i;
	struct SentenceWord *word;
	struct NodeToken *arg;
	struct NodeToken *node;

	foreach_begin(word, sent->words);
	printf("#INFO WORD %d. %s\n", i, vs(word->word));
	for (word = word->or_word; word; word = word->or_word)
		printf("#INFO WORD %d. | %s\n", i, vs(word->word));
	foreach_end;

	foreach_begin(arg, sent->args);
	printf("#INFO ARG %d. %s\n", i, vs(arg->token));
	foreach_end;

	for (node = sent->body->fst; node; node = node->next) {
		printf("#INFO BODY NODE. %s\n", vs(node->token));
	}
}
