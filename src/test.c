#include <stdio.h>
#include <unistd.h>

char *clear_str = "\x1B[2J\x1B[H";
char *eee = "Эээ world\n";

char *life = "██";
char *dead = "__";
#define size 16
char field[16][16] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	
	{0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
};
char evolution[16][16] = {0};

void write_field() {
	int x, y;

	for (x = 0; x < size; x++) {
		for (y = 0; y < size; y++) {
			printf("%s", field[x][y] ? life : dead);
		}
		printf("%s", eee);
	}
}

int live_near(int x, int y) {
	int i, j, xi, yj, live;
	live = 0;

	for (i = -1; i < 2; i++) {
		for (j = -1; j < 2; j++) {
			if (!i && !j)
				break;

			xi = x + i, yj = y + j;
			live += field
				[xi < 0 ? size-1 : xi < size ? xi : 0]
				[yj < 0 ? size-1 : yj < size ? yj : 0]
			;
		}
	}
	return live;
}

void to_live() {
	int x, y, live;

	for (x = 0; x < size; x++) {
		for (y = 0; y < size; y++) {
			live = live_near(x, y);
			evolution[x][y] = field[x][y] ? live == 2 || live == 3 : live == 3;
		}
	}
	for (x = 0; x < size; x++) {
		for (y = 0; y < size; y++) {
			field[x][y] = evolution[x][y];
		}
	}
}

int main () {
	while (1) {
		write_field();
		to_live();
		usleep(500000);
	}
	return 77;
}
