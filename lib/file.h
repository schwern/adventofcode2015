#include <stdbool.h>
#include <stdio.h>

#ifdef DEBUG
#  define DEBUG 1
#else
#  define DEBUG 0
#endif

FILE *open_file(const char *filename, const char *mode);

void usage(int argc, char *desc[]);
