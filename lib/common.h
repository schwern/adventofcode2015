#include <stdbool.h>
#include <stdio.h>
#include <glib.h>

#ifdef DEBUG
#  define DEBUG 1
#else
#  define DEBUG 0
#endif

FILE *open_file(const char *filename, const char *mode);

void usage(int argc, char *desc[]);

// Same as g_regex_new, but no error
GRegex *compile_regex(
    const gchar *pattern,
    GRegexCompileFlags compile_options,
    GRegexMatchFlags match_options
);
