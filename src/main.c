#include "gner.h"
#include <stdio.h>

uc NEED_WARN = 1;

int main() {
	char *filename = "тест.ср";
	char *outname = "тест.асм";

	struct Pser *p = new_pser(filename, 1);
	struct Gner *g = new_gner(p, T_Асм_Linux_64, 1);
	gen(g);

	long bytes = g->bprol->size + g->prol->size + g->text->size;
	FILE *f = fopen(outname, "wb");
	fwrite(g->bprol->st, 1, g->bprol->size, f);
	fwrite(g->prol->st, 1, g->prol->size, f);
	fwrite(g->aprol->st, 1, g->aprol->size, f);
	fwrite(g->text->st, 1, g->text->size, f);
	fclose(f);

	printf("В файл [%s] записано %ld байт", outname, bytes);
	int end = bytes % 10;
	if (end >= 2 && end <= 4)
		printf("а");
	printf(" текста\n");

	return 0;
}
