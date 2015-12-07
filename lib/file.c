#include <stdbool.h>
#include <stdio.h>
#include "file.h"

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
