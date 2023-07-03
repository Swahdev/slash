#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\e[1;34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define RED "\e[1;91m"
#define GRN "\e[1;92m"

#define MAX_LENGTH_LINE 30
#define COLOR_LENGTH 10

extern void parse(char *line, char sep, char **tab, int *tabLength, EV *ev);
extern void extract(char *line, char *path, unsigned int number);
int parsePipe(char **tab, int argc, EV *ev);
void terminate(EV *ev, char **tab);