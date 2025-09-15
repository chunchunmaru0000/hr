#include "prep.h"
#include <stdio.h>

// ####################################################################
// 						BELOW IS SENTENCE CALL GEN
// ####################################################################

// ####################################################################
// 						BELOW IS SENTENCE CALL TRY
// ####################################################################

const char *const EXPECTED_COMMA_AFTER_SENT_WITH_END_ON_ARG =
	"Если буки кончаются на аргумент, то при вызове таких бук после последнего "
	"аргумента должна стоять запятая, чтобы ограничивать данный аргумент.";

int cmp_sent_word(struct SentenceWord *w, struct Token *token) {
	while (w) {
		if (vc(w->word, token))
			return 1;
		w = w->or_word;
	}
	return 0;
}

int cmp_words_until(uint32_t *from, uint32_t to, struct Sentence *sentence,
					struct NodeToken **n) {
	for (; *from < to; (*from)++) {
		// printf("#INFO. try_apply_sentence №%d [%s]\n", i, vs(n->token));
		if (!cmp_sent_word(plist_get(sentence->words, *from), (*n)->token))
			return 0;
		*n = take_guaranteed_next(*n);
	}
	return 1;
}

struct PList *get_sent_args_as_plist_of_node_tokens(struct Sentence *s) {
	struct PList *node_args = new_plist(s->args->size);
	struct SentenceArg *s_arg;
	uint32_t i = 0;
	foreach_by(i, s_arg, s->args);
	plist_set(node_args, i, s_arg->token);
	foreach_end;
	return node_args;
}

void free_args_nodes_not_copyed(struct PList *args_nodes) {
	struct Nodes *arg_nodes;
	uint32_t i;
	foreach_begin(arg_nodes, args_nodes);
	free(arg_nodes);
	foreach_end;
	plist_free(args_nodes);
}
#define exit_zero_with_freed_args_nodes()                                      \
	do {                                                                       \
		free_args_nodes_not_copyed(args_nodes);                                \
		return 0;                                                              \
	} while (0)

int try_apply_with_args_sentence(struct Sentence *sentence,
								 struct NodeToken **cur) {
	struct NodeToken *c = *cur, *n, *snd_word, *lst_word;
	struct PList *node_args, *args_nodes = new_plist(sentence->args->size);
	struct Nodes *body, *arg_nodes;
	struct SentenceArg *arg, *next_arg = plist_get(sentence->args, 0);
	uint32_t i, j = 1; // start index of cmp cuz zero word is already compared
	struct SentenceWord *word_after_arg;

	n = take_guaranteed_next(c);
	snd_word = n;

	for (i = 0; /* i < sentence->args->size */; i++) {
		arg = next_arg;

		if (!cmp_words_until(&j, arg->index, sentence, &n))
			exit_zero_with_freed_args_nodes();
		// here all words before this arg are valid so can just parse arg nodes

		// here will just get args nodes and only after copy them it saves
		// malloc and free time in case if return that is longer than just
		// second loop for args usages
		if (i + 1 >= sentence->args->size) { // means last arg
			// here need to parse last arg
			arg_nodes = new_nodes(n, 0);
			plist_add(args_nodes, arg_nodes);

			if (arg->index + 1 >= sentence->words->size) {
				if (n->token->code == COMMA)
					eet(n->token, CANT_HAVE_EMPTY_ARG_YET, 0);

				while (n->token->code != COMMA)
					n = nol_with_err(arg_nodes->fst, n,
									 EXPECTED_COMMA_AFTER_SENT_WITH_END_ON_ARG);

				arg_nodes->lst = n->prev;
			} else {
				word_after_arg = plist_get(sentence->words, arg->index + 1);

				if (cmp_sent_word(word_after_arg, n->token))
					eet(n->token, CANT_HAVE_EMPTY_ARG_YET, 0);
				// cuz can assume its not equal with word_after_arg
				n = next_of_line(arg_nodes->fst, n);

				while (!cmp_sent_word(word_after_arg, n->token))
					n = next_of_line(arg_nodes->fst, n);

				arg_nodes->lst = n->prev;

				// TODO: i beleive its 2 cuz last word is already compared in
				// while loop above
				j = arg->index + 2;
				// TODO: here may be some shin in cmp_words_until with last n so
				if (!cmp_words_until(&j, sentence->words->size, sentence, &n))
					exit_zero_with_freed_args_nodes();
			}
			break; // exits loop

		} else {
			word_after_arg = plist_get(sentence->words, arg->index + 1);
			next_arg = plist_get(sentence->args, i + 1);
		}
	}

	// TODO: copy all arg_nodes in args_nodes

	lst_word = n == snd_word ? snd_word : n->prev;
	c = cut_off_inclusive(snd_word, lst_word);

	node_args = get_sent_args_as_plist_of_node_tokens(sentence);
	body = gen_body(sentence->body, node_args, args_nodes);
	*cur = replace_nodes_inclusive(c, body);

	plist_free(node_args);
	return 1;
}

int try_apply_without_args_sentence(struct Sentence *sentence,
									struct NodeToken **cur) {
	struct NodeToken *c = *cur;
	struct Nodes *body;
	struct NodeToken *n, *snd_word, *lst_word;
	uint32_t i = 1;

	n = take_guaranteed_next(c);
	snd_word = n;

	if (!cmp_words_until(&i, sentence->words->size, sentence, &n))
		return 0;

	lst_word = n == snd_word ? snd_word : n->prev;
	c = cut_off_inclusive(snd_word, lst_word);
	body = copy_nodeses(0, sentence->body);
	*cur = replace_nodes_inclusive(c, body);

	return 1;
}

int try_apply_sentence(struct Prep *pr, struct NodeToken **cur) {
	struct NodeToken *c = *cur;
	struct Sentence *sentence;
	uint32_t i;

	foreach_by(i, sentence, pr->sentences);
	// printf("#INFO. try_apply_sentence [%s]\n", vs((*cur)->token));
	if (!cmp_sent_word(plist_get(sentence->words, 0), c->token))
		continue;

	if (sentence->args->size ? try_apply_with_args_sentence(sentence, cur)
							 : try_apply_without_args_sentence(sentence, cur))
		return 1;
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

void parse_sent_args_and_words(struct Sentence *sent, struct NodeToken **name) {
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

		plist_add(words, 0); // as arg index filler
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

	parse_sent_args_and_words(sent, &c);
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
