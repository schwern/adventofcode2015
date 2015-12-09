#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>
#include "file.h"

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
