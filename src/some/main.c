#include "../gner/gner.h"
#include <stdio.h>

uc NEED_WARN = 1;

#define g_write(name) (fwrite(g->name->st, 1, g->name->size, file))

int main() {
	char *filename = "тест.ср";
	char *outname = "тест.асм";

	struct Tzer *t = new_tzer(filename);
	struct Fpfc *f = t->f;
	struct PList *tokens = preprocess(t); // tzer freed in here

	struct Pser *p = new_pser(f, tokens, filename, 1);
	struct Gner *g = new_gner(p, T_Асм_Linux_64, 1);
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
