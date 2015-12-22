#ifndef _common_h
#define _common_h

#include <stdbool.h>
#include <stdio.h>
#include <glib.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef DEBUG
#  define DEBUG 1
#else
#  define DEBUG 0
#endif

#define TWOD(array, x, y, y_size) array[x + (y*y_size)]

typedef void (*LineCB)(char *line, void *cb_data);

FILE *open_file(const char *filename, const char *mode);

void foreach_line(FILE *line, LineCB cb, void *cb_data);

void usage(int argc, char *desc[]);

void die(char *format, ...);

// Same as g_regex_new, but no error
GRegex *compile_regex(
    const gchar *pattern,
    GRegexCompileFlags compile_options,
    GRegexMatchFlags match_options
);

static inline int ipow(int base, int power) {
    return pow(base, power);
}

static inline bool is_empty(const char *str) {
    return str[0] == '\0';
}

static inline bool streq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

char *num_to_str(long num);

#endif
