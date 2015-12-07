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

bool usage(const int argc, const int argc_desc, char **argv_desc) {
    if( argc != argc_desc ) {
        fputs("Usage: ", stderr);
        for(int i = 1; i < argc_desc; i++) {
            fprintf(stderr, " %s", argv_desc[i]);
        }
        fprintf(stderr, "\n");

        return false;
    }

    return true;
}
