#include "file.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

static bool has_vowels(char *orig, const int want) {
    int seen = 0;
    char *string = orig;

    if( strlen(string) < want )
        return false;
    
    while( (string = strpbrk(string, "aeiou")) != NULL ) {
        seen++;
        if( seen >= want )
            break;
        string++;
    }

    return seen >= want;
}

static bool is_nice(char *string) {    
    /* It contains at least three vowels (aeiou only),
     * like aei, xazegov, or aeiouaeiouaeiou. */
    if( !has_vowels(string, 3) )
        return false;

    bool seen_double = false;
    for(int i = 0; string[i+1] != '\0'; i++) {
        char next = string[i+1];

        /* - It contains at least one letter that appears twice in a row, like xx, abcdde (dd),
         *   or aabbccdd (aa, bb, cc, or dd). */
        if( string[i] == next ) {
            seen_double = true;
        }

        /* - It does not contain the strings ab, cd, pq, or xy, even if they are part of one of
         *   the other requirements. */
        switch(string[i]) {
            case 'a':
                if( next == 'b' )
                    return false;
                break;

            case 'c':
                if( next == 'd' )
                    return false;
                break;

            case 'p':
                if( next == 'q' )
                    return false;
                break;

            case 'x':
                if( next == 'y' )
                    return false;
                break;
        }
    }

    return seen_double;
}

static int count_nice( FILE *fp ) {
    char *line = NULL;
    size_t line_len = 0;
    int num_nice = 0;
    
    while( getline(&line, &line_len, fp) > 0 ) {
        if( is_nice(line) )
            num_nice++;
    }
    free(line);

    return num_nice;
}

int main(const int argc, char **argv) {
    char *argv_desc[2] = {argv[1], "<input file>"};
    
    if( !usage(argc, 2, argv_desc) ) {
        return -1;
    }

    FILE *fp = open_file(argv[1], "r");

    int num_nice = count_nice(fp);
    printf("%d\n", num_nice);

    return 0;
}
