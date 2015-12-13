#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>
#include "common.h"

FILE *open_file(const char *filename, const char *mode) {
    FILE *fp = fopen(filename, mode);
    if( fp == NULL ) {
        fprintf(stderr, "Could not open %s: %s.\n", filename, strerror(errno));
        exit(errno);
    }

    return fp;
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
    GError *error;

    GRegex *re = g_regex_new(pattern, compile_options, match_options, &error);

    if( error != NULL ) {
        fprintf(stderr, "Can't compile regex: %s\n", error->message);
        g_error_free(error);
        exit(1);
    }

    return re;
}

void die(char *format, ...) {
    va_list args;
    
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    exit(1);
}
