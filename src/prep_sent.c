#include "prep.h"
#include <stdio.h>

// ####################################################################
// 						BELOW IS SENTENCE CALL GEN
// ####################################################################

// ####################################################################
// 						BELOW IS SENTENCE CALL TRY
// ####################################################################

int cmp_sent_word(struct SentenceWord *w, struct Token *token) {
	while (w) {
		if (vc(w->word, token))
			return 1;
		w = w->or_word;
	}
	return 0;
}

int try_apply_sentence(struct Prep *pr, struct NodeToken **cur) {
	struct NodeToken *c = *cur;
	struct SentenceWord *sent_word;
	struct SentenceArg *sent_arg;
	struct Sentence *sentence;
	struct Nodes *body;
	struct NodeToken *n, *snd_word, *lst_word;
	uint32_t i, j;

	foreach_by(i, sentence, pr->sentences);
	sent_word = plist_get(sentence->words, 0);

	if (cmp_sent_word(sent_word, c->token)) {
		if (sentence->args->size) {
			exit(220);

		} else { // here just all words should be equal
			n = take_guaranteed_next(c);
			snd_word = n;

			for (j = 1; j < sentence->words->size; j++) {
				if (!cmp_sent_word(plist_get(sentence->words, j), n->token))
					break;
				n = take_guaranteed_next(n);
			}
			if (j != sentence->words->size)
				continue;

			lst_word = n == snd_word ? snd_word : n->prev;
			c = cut_off_inclusive(snd_word, lst_word);
			body = copy_nodeses(0, sentence->body);
			*cur = replace_nodes_inclusive(c, body);
			return 1;
		}
	}

	foreach_end;

	return 0;
}

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

int until_next_arg(struct PList *words, struct NodeToken **c, uint32_t *i) {
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
		(*i)++;
		*c = n;
	}
}

void parse_sent_args_and_words(struct Prep *pr, struct Sentence *sent,
							   struct NodeToken **name) {
	struct PList *args = new_plist(8);
	struct PList *words = new_plist(16);
	struct SentenceArg *arg;
	struct NodeToken *c = take_guaranteed_next(*name);
	uint32_t i;

	if (c->token->code != SH_L)
		eet(c->token, EXPECTED_SENT_START_SH_L, 0);

	for (i = 0; until_next_arg(words, &c, &i); i++) {
		c = take_guaranteed_next(c);
		if (c->token->code != ID)
			eet(c->token, EXPECTED_ID_AS_MACRO_NAME, 0);

		arg = malloc(sizeof(struct SentenceArg));
		arg->index = i;
		arg->token = deep_clone_token(c->token, c->token->p);
		plist_add(args, arg);

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
void assert_sent(struct Sentence *sent, struct NodeToken *name);
const char *const SENTENCE_CANT_HAVE_LESS_THAN_2_WORDS =
	"Буки не могут иметь в себе менее двух слов, не считая аргументы.";
const char *const SENTENCE_CANT_START_WITH_ARG =
	"Буки не могут начинаться с аргумента.";
const char *const TWO_ARGS_NEED_TO_BE_SEPARATED =
	"Два аргумента в буках должны разделяться хотя бы одним словом, так как "
	"слова являются разграничителями аргументов.";

// TODO: need to so most late first args sents are first to compare
//	sents
//	|> sort fun sent -> sent->args[0]->index end
//	|> reverse
// and it will guarantee that is there is sents that start with same words them
// longest ones can always be possible and no need to declare them before
struct NodeToken *parse_sent(struct Prep *pr, struct NodeToken *name) {
	struct NodeToken *c = name;
	struct Sentence *sent = malloc(sizeof(struct Sentence));
	plist_add(pr->sentences, sent);

	parse_sent_args_and_words(pr, sent, &c);
	sent->body = parse_body(&c);

	assert_sent(sent, name);
	// debug_sent(sent);

	return c;
}

void assert_sent(struct Sentence *sent, struct NodeToken *name) {
	struct SentenceArg *sent_arg, *tmp_arg;
	uint32_t i;

	if (sent->words->size < 2)
		eet(name->token, SENTENCE_CANT_HAVE_LESS_THAN_2_WORDS, 0);
	if (sent->args->size) {
		sent_arg = plist_get(sent->args, 0);

		if (sent_arg->index == 0)
			eet(sent_arg->token, SENTENCE_CANT_START_WITH_ARG, 0);

		for (i = 1; i < sent->args->size; i++) {
			tmp_arg = plist_get(sent->args, i);
			if (tmp_arg->index - sent_arg->index < 2)
				eet(tmp_arg->token, TWO_ARGS_NEED_TO_BE_SEPARATED, 0);
			sent_arg = tmp_arg;
		}
	}
}

void debug_sent(struct Sentence *sent) {
	uint32_t i;
	struct SentenceWord *word;
	struct SentenceArg *arg;
	struct NodeToken *node;

	foreach_begin(word, sent->words);
	printf("#INFO WORD %d. %s\n", i, vs(word->word));
	for (word = word->or_word; word; word = word->or_word)
		printf("#INFO WORD %d. | %s\n", i, vs(word->word));
	foreach_end;

	foreach_begin(arg, sent->args);
	printf("#INFO ARG %d. %s with index %d\n", i, vs(arg->token), arg->index);
	foreach_end;

	for (node = sent->body->fst; node; node = node->next) {
		printf("#INFO BODY NODE. %s\n", vs(node->token));
	}
}
