#include "pser.h"
#include <stdio.h>

void ee_token(struct Token *t, char *msg) { // error exit
	fprintf(stderr, "%s%s:%d:%d:%s ОШИБКА: %s [%s]:[%d]%s\n", COLOR_WHITE,
			t->p->f->path, t->p->line, t->p->col, COLOR_RED, msg, t->view->st,
			t->code, COLOR_RESET);
	print_source_line(t->p, COLOR_LIGHT_RED, 0);
	exit(1);
}

// print warning
void pw(struct Token *t, constr msg, constr sgst) {
	if (!NEED_WARN)
		return;

	char *help;
	uint32_t token_chars_len, help_len, sgst_len = -1;
	if (sgst)
		sgst_len = strlen(sgst);

	token_chars_len =
		get_utf8_chars_to_pos((const char *)t->view->st, t->view->size);

	help_len = token_chars_len + 1 + sgst_len;
	help = malloc(help_len + 1);

	help[help_len] = 0; // terminate
	for (uint32_t i = 0; i < token_chars_len; i++)
		help[i] = UNDERLINE_CHAR; // fill with underline

	if (sgst) {
		help[token_chars_len] = '\n'; // split by \n
		memcpy(help + token_chars_len + 1, sgst, sgst_len);
	}

	fprintf(stderr, "%s%s:%d:%d%s ПРЕДУПРЕЖДЕНИЕ: %s%s\n", COLOR_WHITE,
			t->p->f->path, t->p->line, t->p->col, COLOR_LIGHT_PURPLE, msg,
			COLOR_RESET);
	print_source_line(t->p, COLOR_LIGHT_PURPLE, help);
}

struct BList *get_extra_sgst(struct ErrorInfo *info) {
	struct BList *sgst_list, *tmp_list;
	long sgst_len;

	sgst_len = strlen(info->sgst);
	sgst_list = new_blist(strlen(info->sgst));
	blat(sgst_list, (uc *)info->sgst, sgst_len);

	if (info->extra_type == ET_INT) {
		tmp_list = int_to_str((long)info->extra);
		blat_blist(sgst_list, tmp_list);
		blist_add(sgst_list, '\0');

		blist_clear_free(tmp_list);
	} else
		exit(227);

	return sgst_list;
}

void pwei_with_extra(struct ErrorInfo *info) {
	if (info->extra_type == ET_NONE) {
		pw(info->t, info->msg, info->sgst);
		return;
	}

	struct BList *sgst_list = get_extra_sgst(info);

	pw(info->t, info->msg, (char *)sgst_list->st);
	blist_clear_free(sgst_list);
}

void etei_with_extra(struct ErrorInfo *info) {
	if (info->extra_type == ET_NONE) {
		etei(info);
		return;
	}

	struct BList *sgst_list = get_extra_sgst(info);

	et(info->t, info->msg, (char *)sgst_list->st);
	blist_clear_free(sgst_list);
}

void eei(struct Inst *in, constr msg, constr sgst) {
	eet(in->start_token, msg, sgst);
}

void pser_err(struct Pser *p) {
	struct ErrorInfo *ei;
	int i;

	for (i = p->errors->size - 1; i >= 0; i--) {
		ei = plist_get(p->errors, i);
		etei_with_extra(ei);
	}

	for (i = p->warns->size - 1; i >= 0; i--) {
		ei = plist_get(p->warns, i);
		pwei_with_extra(ei);
		free(ei);
	}
	plist_clear(p->warns);

	if (p->errors->size)
		exit(1);
}
