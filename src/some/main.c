#include "../gner/gner.h"
#include <stdio.h>

struct Gner *ogner;
uc NEED_WARN = 1;

void get_file_in_and_out_names(int argov, char **args, char **in, char **out) {
	if (argov < 3 || argov > 4) {
		printf("%sОжидалось от 2-х до 3-х аргументов, возможные варианты:\n%s",
			   COLOR_RED, COLOR_RESET);
		printf("%s * |%s исток исход [-бп -без_предупреждений] %s\n",
			   COLOR_RESET, COLOR_GREEN, COLOR_RESET);
		exit(1);
	}
	struct PList *argi = new_plist(argov);
	struct BList *arg;
	u32 i;

	for (i = 0; i < (u32)argov; i++) {
		arg = zero_term_blist(copy_blist_from_str(args[i]));
		plist_add(argi, arg);
	}
	for (i = 3; i < argi->size; i++) {
		arg = plist_get(argi, i);

		if (sc(bs(arg), "-бп") || sc(bs(arg), "-без_предупреждений")) {
			NEED_WARN = 0, blist_clear_free(arg);
			continue;
		}

		printf("%sНеизвестный аргумент: %s%s\n", COLOR_RED, bs(arg),
			   COLOR_RESET);
		exit(1);
	}
	arg = plist_get(argi, 1), *in = bs(arg);
	arg = plist_get(argi, 2), *out = bs(arg);
	plist_free(argi);
}

#define g_write(name) (fwrite(g->name->st, 1, g->name->size, file))

int main(int argov, char **args) {
	char *filename, *outname;
	get_file_in_and_out_names(argov, args, &filename, &outname);

	struct Tzer *t = new_tzer(filename);
	struct Fpfc *f = t->f;
	struct PList *tokens = preprocess(t); // tzer freed in here

	struct Pser *p = new_pser(f, tokens, filename, 1);
	struct Gner *g = new_gner(p, T_Асм_Linux_64, 1);
	ogner = g;
	gen(g);

	long bytes =
		g->bprol->size + g->prol->size + g->aprol->size + g->text->size;
	FILE *file = fopen(outname, "wb");
	g_write(bprol);
	g_write(prol);
	g_write(aprol);
	g_write(text);
	fclose(file);

	printf("В файл [%s] записано %ld байт", outname, bytes);
	int end = bytes % 10;
	if (end >= 2 && end <= 4)
		printf("а");
	printf(" текста\n");

	return 0;
}
