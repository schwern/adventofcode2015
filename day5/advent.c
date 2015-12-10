#include "common.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

GError *ReError;
GRegex *ReRepeat;
GRegex *RePair;

static void init_regexes() {
    RePair = g_regex_new("(.).\\1", G_REGEX_OPTIMIZE, 0, &ReError);
    ReRepeat = g_regex_new("((.)[^\\2]).*\\1", G_REGEX_OPTIMIZE, 0, &ReError);
}

static bool is_nice(char *string) {
    /* It contains at least one letter which repeats with exactly one letter
       between them, like xyx, abcdefeghi (efe), or even aaa. */
    if( !g_regex_match(ReRepeat, string, 0, NULL ) )
        return false;

    /* It contains a pair of any two letters that appears at least twice in
       the string without overlapping, like xyxy (xy) or aabcdefgaa (aa), but
       not like aaa (aa, but it overlaps). */
    if( !g_regex_match(RePair, string, 0, NULL ) )
        return false;
    
    return true;
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
    if( argc != 2 ) {
        char *argv_desc[2] = {argv[1], "<input file>"};
        usage(2, argv_desc);
        return -1;
    }

    FILE *fp = open_file(argv[1], "r");

    init_regexes();
    
    int num_nice = count_nice(fp);
    printf("%d\n", num_nice);

    return 0;
}
