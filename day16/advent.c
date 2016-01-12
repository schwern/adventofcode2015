#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <glib.h>
#include <stdlib.h>

GRegex *Line_Re;

static void free_regexes() {
    g_regex_unref(Line_Re);
}

static void init_regexes() {
    if( !Line_Re ) {
        Line_Re = compile_regex(
            "^"
            "Sue (?<NUM>\\d+): (?<PROPS>.+)"
            "$",
            G_REGEX_OPTIMIZE,
            0
        );
        atexit(free_regexes);
    }
}

static bool filter_aunt(char *prop_line, GHashTable *filter) {
    gchar **props = g_regex_split_simple(", ", prop_line, 0, 0);

    bool check = true;
    for( int i = 0; props[i] != NULL; i++ ) {
        gchar **tuple = g_regex_split_simple(": ", props[i], 0, 0);
        char *key   = tuple[0];
        char *_have = tuple[1];
        int have = atoi(_have);

        if( DEBUG )
            printf("Trying %s -> %d\n", key, have);
        
        void *_want = g_hash_table_lookup( filter, key );
        int want = *(int *)_want;

        if( streq(key, "cats") || streq(key, "trees") ) {
            if( have <= want )
                check = false;
        }
        else if( streq(key, "pomeranians") || streq(key, "goldfish") ) {
            if( have >= want )
                check = false;
        }
        else {
            if( have != want )
                check = false;
        }
        
        g_strfreev(tuple);

        if( !check )
            break;
    }

    g_strfreev(props);
    
    return check;
}

static int find_aunt(FILE *input, GHashTable *filter) {
    char *line = NULL;
    size_t linecap = 0;

    GMatchInfo *match;
    int matched_aunt = 0;
    
    while( getline(&line, &linecap, input) > 0 ) {
        if( g_regex_match(Line_Re, line, 0, &match ) ) {
            char *props = g_match_info_fetch_named(match, "PROPS");
            
            if( filter_aunt(props, filter) ) {
                char *num = g_match_info_fetch_named(match, "NUM");
                matched_aunt = atoi(num);
                free(num);
            }

            free(props);

            if( matched_aunt )
                break;
        }
        else if( !is_blank(line) ) {
            die("Unknown line: '%s'\n", line);
        }

        g_match_info_free(match);
    }

    free(line);
    g_match_info_free(match);
    
    return matched_aunt;
}

static inline void add_to_filter(GHashTable *filter, char *key, int val) {
    int *val_p = malloc(sizeof(val));
    *val_p = val;

    g_hash_table_insert(filter, key, val_p);
}

static GHashTable *make_filter() {
    GHashTable *filter = g_hash_table_new_full( g_str_hash, g_str_equal, NULL, free );

    add_to_filter(filter, "children", 3);
    add_to_filter(filter, "cats", 7);
    add_to_filter(filter, "samoyeds", 2);
    add_to_filter(filter, "pomeranians", 3);
    add_to_filter(filter, "akitas", 0);
    add_to_filter(filter, "vizslas", 0);
    add_to_filter(filter, "goldfish", 5);
    add_to_filter(filter, "trees", 3);
    add_to_filter(filter, "cars", 2);
    add_to_filter(filter, "perfumes", 1);

    return filter;
}

int main(int argc, char *argv[]) {
    init_regexes();
    
    if( argc == 2 ) {
        GHashTable *filter = make_filter();
        
        FILE *input = open_file(argv[1], "r");
        printf("%d\n", find_aunt(input, filter));

        g_hash_table_unref(filter);
    }
    else {
        char *desc[] = {argv[0], "<inputfile>"};
        usage(2, desc);
    }
}
