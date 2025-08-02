#include "tpes.h"
#include <string.h>

#define loop while (1)
#define loa(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define sc(view, str) (strcmp((view), (str)) == 0)

extern const char *const COLOR_BLACK;
extern const char *const COLOR_RED;
extern const char *const COLOR_LIGHT_RED;
extern const char *const COLOR_GREEN;
extern const char *const COLOR_YELLOW;
extern const char *const COLOR_BLUE;
extern const char *const COLOR_PURPLE;
extern const char *const COLOR_LIGHT_PURPLE;
extern const char *const COLOR_GAY;
extern const char *const COLOR_WHITE;
extern const char *const COLOR_RESET;
extern const char UNDERLINE_CHAR;
#define color_print(c, msg) (printf("%s%s%s", (c), (msg), COLOR_RESET))
uint32_t get_utf8_chars_to_pos(const char *str, int col);

struct Tzer {
	struct Fpfc *f;
	struct Pos *p;
	size_t pos;
};

enum ExtraType {
	ET_NONE,
	ET_INT,
};

struct ErrorInfo {
	struct Fpfc *f;
	struct Token *t;
	const char *msg;
	const char *sgst;

	void *extra;
	enum ExtraType extra_type;
};
struct ErrorInfo *new_error_info(struct Fpfc *f, struct Token *t,
								 const char *const msg, const char *const sgst);

struct Tzer *new_tzer(char *);
struct Token *new_token(struct Tzer *);
struct PList *tze(struct Tzer *, long);
void print_source_line(const char *, struct Pos *, const char *const, char *);
void ee(struct Fpfc *, struct Pos *, const char *const);
void et(struct Fpfc *f, struct Token *t, const char *const msg,
		const char *const sgst);
void eet(struct Fpfc *f, struct Token *t, const char *const msg,
		 const char *const sgst);
#define etei(ei) (et((ei)->f, (ei)->t, (ei)->msg, (ei)->sgst))
