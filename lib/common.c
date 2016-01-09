#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>
#include <math.h>
#include "common.h"

FILE *open_file(const char *filename, const char *mode) {
    FILE *fp = fopen(filename, mode);
    if( fp == NULL ) {
        fprintf(stderr, "Could not open %s: %s.\n", filename, strerror(errno));
        exit(errno);
    }

    return fp;
}

void foreach_line(FILE *fp, LineCB cb, void *cb_data) {
    char *line = NULL;
    size_t line_len = 0;

    while( getline(&line, &line_len, fp) > 0 ) {
        cb(line, cb_data);
    }

    free(line);
}

void usage(int argc, char *desc[]) {
    fputs("Usage:", stderr);
    for( int i = 0; i < argc; i++ ) {
        fprintf(stderr, " %s", desc[i]);
    }
    fputs("\n", stderr);
}

// Same as g_regex_new, but no error
GRegex *compile_regex(
    const gchar *pattern,
    GRegexCompileFlags compile_options,
    GRegexMatchFlags match_options
) {
    GError *error = NULL;

    GRegex *re = g_regex_new(pattern, compile_options, match_options, &error);

    if( error != NULL ) {
        fprintf(stderr, "Can't compile regex: %s\n", error->message);
        g_error_free(error);
        exit(1);
    }

    return re;
}

bool is_blank(char *line) {
    static GRegex *blank_line_re = NULL;

    if( !blank_line_re ) {
        blank_line_re = compile_regex(
            "^ \\s* $",
            G_REGEX_OPTIMIZE | G_REGEX_EXTENDED,
            0
        );
    }

    return g_regex_match(blank_line_re, line, 0, NULL);
}

void die(char *format, ...) {
    va_list args;
    
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    exit(1);
}


char *num_to_str(long num) {
    int len = num == 0 ? 1 : floor(log10l(labs(num)))+1;

    /* Add room for a negative sign */
    if( num < 0 )
        len++;
    
    char *result = calloc(len+1, sizeof(char));
    snprintf(result, len+1, "%ld", num);

    return result;
}
