#include "prep.h"

const char *const EXPECTED_STR_AS_AN_INCLUDE_PATH =
	"После инструкции препроцессора '#влечь' ожидалась строка содержащая путь "
	"файла.";

#define ALREADY_INCLUDED -1
const char *const STR_INCLUDE = "влечь";

struct BList *try_include_path(struct Token *path) {
	struct BList *included_path;
	uint32_t i;

	foreach_begin(included_path, included_files);
	// here compare token str cuz view has "
	if (sc(ss(path), (char *)included_path->st))
		return 0;
	foreach_end;

	included_path = path->str;
	plist_add(included_files, included_path);

	return included_path;
}

struct NodeToken *get_included_head(struct BList *included_path) {
	struct NodeToken *included_head;
	struct PList *new_tokens;
	struct Tzer *tzer;

	tzer = new_tzer((char *)included_path->st);
	new_tokens = tze(tzer, 128);
	// printf("# gen_node_tokens in %s with %d num of tokens\n",
	//         (char *)included_path->st, new_tokens->size);
	included_head = gen_node_tokens(new_tokens);

	free(tzer);
	plist_free(new_tokens); // here every EF token is lost
	return included_head;
}

struct NodeToken *try_include(struct NodeToken *prev, struct Token *path) {
	struct NodeToken *included_head;

	struct BList *path_to_include = try_include_path(path);
	if (!path_to_include)
		return (void *)ALREADY_INCLUDED;

	included_head = get_included_head(path_to_include);
	// printf("## got included_head with token %s, prev now is %p\n",
	//         vs(included_head->token), prev);
	if (included_head->token->code == EF)
		return prev;

	included_head->prev = prev;
	if (prev)
		prev->next = included_head;

	// here need to return tail, cuz head already attached to the prev
	// so it will proceed gen loop from the tail
	// also need to skip EOF token in here
	// printf("## in %s with head of %s and next %s\n", vs(path),
	//         vs(included_head->token), vs(included_head->next->token));
	while (included_head->next && included_head->token->code != EF)
		included_head = included_head->next;
	// printf("### after a while with head of %s and next %p\n",
	//         vs(included_head->token), included_head->next);

	if (included_head->token->code == EF) {
		// printf("#### in %s returns head of %s\n", vs(path),
		//         vs(included_head->prev->token));
		// TODO: free new_head->new_head and its token
		return included_head->prev;
	} else
		exit(19);
}

struct NodeToken *try_parse_include(struct NodeToken *prev,
									struct PList *tokens, uint32_t i) {
	struct Token *token;

	token = plist_get(tokens, i);
	if (token->code != SHARP || i + 2 >= tokens->size)
		return 0;

	token = plist_get(tokens, ++i);
	if (token->code != ID || !vcs(token, STR_INCLUDE))
		return 0;

	token = plist_get(tokens, ++i);
	if (token->code != STR)
		eet(token, EXPECTED_STR_AS_AN_INCLUDE_PATH, 0);

	return try_include(prev, token);
}

struct NodeToken *gen_node_tokens(struct PList *tokens) {
	struct NodeToken *head; // = malloc(sizeof(struct NodeToken));
	struct NodeToken *cur = head, *prev = 0;
	uint32_t i;

	for (i = 0; i < tokens->size; i++) {
		cur = try_parse_include(prev, tokens, i);

		if (cur == 0)
			goto just_token;
		if (cur == (void *)ALREADY_INCLUDED)
			goto skip_include;
		if (cur) {
			// printf("##### included with cur of %s\n", vs(cur->token));
			if (prev == 0)
				head = cur;
			else if (prev != cur)
				prev->next = cur;
			// printf("##### head now is %s\n", vs(head->token));
			prev = cur;
		skip_include:
			i += 2; // skip 'влечь' and string path literal
			continue;
		}

	just_token:
		cur = malloc(sizeof(struct NodeToken));
		cur->token = plist_get(tokens, i);
		cur->prev = prev;
		if (prev == 0)
			head = cur;
		else
			prev->next = cur;
		prev = cur;
	}
	cur->next = 0;

	// printf("###### returns from gen_node_tokens with head of %s\n",
	//         vs(head->token));
	return head;
}
