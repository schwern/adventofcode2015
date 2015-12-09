#include "file.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GRegex *Circuit_Line_Re;

static void init_regexes() {
    GError *error;
    
    if( !Circuit_Line_Re ) {
        Circuit_Line_Re = g_regex_new(
            " ^ \\s* "
            " (?: "
            "   (?<VAL>[[:alnum:]]+) | "
            "   (?:(?<LVAR>[[:alnum:]]+) \\s+)? (?<CMD>[[:alpha:]]+) \\s+ (?<RVAR>[[:alnum:]]+) "
            " ) "
            " \\s* -> \\s* "
            " (?<ASSIGN>[[:alpha:]]+) "
            " \\s* $ ",
            G_REGEX_OPTIMIZE | G_REGEX_EXTENDED,
            0,
            &error
        );

        if( error != NULL ) {
            fprintf(stderr, "Can't compile regex: %s\n", error->message);
            exit(1);
        }
    }
}

static void free_regexes() {
    g_regex_unref(Circuit_Line_Re);
}

static void print_result_cb(gpointer _key, gpointer _val, gpointer user_data) {
    char *key = (char *)_key;
    int   val = *(int *)_val;

    printf("%s: %d\n", key, val);
}

static void process_circuit_line(GHashTable *result, char *line) {
    GMatchInfo *match;
    if( !g_regex_match(Circuit_Line_Re, line, 0, &match) ) {
        fprintf(stderr, "Cannot understand %s.\n", line);
        return;
    }
    
    char *assign = g_match_info_fetch_named(match, "ASSIGN");
    if( !assign ) {
        fprintf(stderr, "No assignment found for line %s.\n", line);
        return;
    }
    assign = strdup(assign);

    char *cmd = g_match_info_fetch_named(match, "CMD");

    if( DEBUG )
        fprintf(stderr, "Command: '%s'.\n", cmd );

    if( !cmd || cmd[0] == '\0' ) {
        char *strval = g_match_info_fetch_named(match, "VAL");
        if( !strval )
            fprintf(stderr, "Missing value in %s.", line);
        int *val = malloc(sizeof(int));
        *val = atoi(strval);

        g_hash_table_replace(result, assign, val);
    }
    else if( strcmp(cmd, "AND") ) {
    }
    else if( strcmp(cmd, "OR") ) {
    }
    else if( strcmp(cmd, "LSHIFT") ) {
    }
    else if( strcmp(cmd, "RSHIFT") ) {
    }
    else if( strcmp(cmd, "NOT" ) ) {
    }
    else {
        fprintf(stderr, "Unknown command %s for line %s.\n", cmd, line);
        return;
    }

    g_match_info_free(match);
}

static GHashTable *read_circuit(FILE *fp) {
    char *line = NULL;
    size_t line_size = 0;
    GHashTable *result = g_hash_table_new(g_str_hash, g_str_equal);

    init_regexes();
    
    while( getline(&line, &line_size, fp) > 0 ) {
        process_circuit_line(result, line);
    }

    free_regexes();
    
    return result;
}

int main(const int argc, char **argv) {
    FILE *input = stdin;

    if( argc > 2 ) {
        char *argv_desc[2] = {argv[0], "<circuit file>"};
        usage(2, argv_desc);
    }

    if( argc == 2 )
        input = open_file(argv[1], "r");

    GHashTable *result = read_circuit(input);
    g_hash_table_foreach( result, print_result_cb, NULL );
    g_hash_table_unref(result);

    return 0;
}
