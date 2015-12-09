#include "file.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

typedef uint16_t c_signal;

GRegex *Circuit_Line_Re;
GRegex *Blank_Line_Re;

static void init_regexes() {
    if( !Blank_Line_Re ) {
        Blank_Line_Re = compile_regex(
            "^ \\s* $",
            G_REGEX_OPTIMIZE | G_REGEX_EXTENDED,
            0
        );
    }
    
    if( !Circuit_Line_Re ) {
        Circuit_Line_Re = compile_regex(
            " ^ \\s* "
            " (?: "
            "   (?<VAL>[[:digit:]]+) | "
            "   (?:(?<LEFT>[[:alpha:]]+) \\s+)? (?<CMD>[[:alpha:]]+) \\s+ (?<RIGHT>[[:alnum:]]+) "
            " ) "
            " \\s* -> \\s* "
            " (?<ASSIGN>[[:alpha:]]+) "
            " \\s* $ ",
            G_REGEX_OPTIMIZE | G_REGEX_EXTENDED,
            0
        );
    }
}

static void free_regexes() {
    g_regex_unref(Circuit_Line_Re);
}

static void print_result_cb(gpointer _key, gpointer _val, gpointer user_data) {
    char *key    = (char *)_key;
    c_signal val = *(c_signal *)_val;

    printf("%s: %d\n", key, val);
}

static inline c_signal *init_val() {
    return (c_signal *)malloc(sizeof(c_signal));
}

static inline char *get_match(GMatchInfo *match, char *key) {
    char *val = g_match_info_fetch_named(match, key);
    if( !val ) {
        fprintf(stderr, "%s not found in %s\n", key, g_match_info_get_string(match));
        exit(1);
    }

    return val;
}

static inline c_signal *get_var_or_init(GHashTable *result, char *key) {
    c_signal *val = g_hash_table_lookup(result, key);
    if( !val ) {
        val = init_val();
        g_hash_table_insert(result, key, val);
    }

    return val;
}

static inline c_signal *get_var(GHashTable *result, char *key) {
    c_signal *val = g_hash_table_lookup(result, key);
    if( !val ) {
        fprintf(stderr, "Variable %s is undefined.\n", key);
        exit(1);
    }

    return val;
}

static inline void set_var(GHashTable *result, char *key, c_signal *val) {
    if( DEBUG )
        fprintf(stderr, "Setting %s to %d\n", key, (int)*val);
    g_hash_table_replace(result, key, val);
}

static inline void set_var_value(GHashTable *result, char *key, c_signal _val) {
    c_signal *val = init_val();
    *val = _val;
    if( DEBUG )
        fprintf(stderr, "Val set to %d\n", (int)*val);
    set_var(result, key, val);
}

static inline bool is_cmd(char *have, char *want) {
    return strcmp(g_ascii_strup(have, -1), g_ascii_strup(want, -1)) == 0 ? true : false;
}

static void process_circuit_line(GHashTable *result, char *line) {
    GMatchInfo *match;

    if( g_regex_match(Blank_Line_Re, line, 0, NULL) ) {
        if( DEBUG )
            fprintf(stderr, "Blank line.\n");
        return;
    }
    
    if( !g_regex_match(Circuit_Line_Re, line, 0, &match) ) {
        fprintf(stderr, "Cannot understand %s.\n", line);
        return;
    }
    
    char *assign = get_match(match, "ASSIGN");
    assign = strdup(assign);

    char *cmd = g_match_info_fetch_named(match, "CMD");

    if( DEBUG )
        fprintf(stderr, "Command: '%s'.\n", cmd );

    if( !cmd || cmd[0] == '\0' ) {
        char *strval = get_match(match, "VAL");
        set_var_value(result, assign, atoi(strval));
    }
    else if( is_cmd(cmd, "AND") || is_cmd(cmd, "OR") ) {
    }
    else if( is_cmd(cmd, "LSHIFT") || is_cmd(cmd, "RSHIFT") ) {
        c_signal rval = (c_signal)atoi(get_match(match, "RIGHT"));

        char *left = get_match(match, "LEFT");
        c_signal *lvalp = get_var(result, left);

        if( DEBUG )
            fprintf(stderr, "%s %d XSHIFT %d -> %s\n", left, *lvalp, rval, assign);

        if( is_cmd(cmd, "LSHIFT") ) {
            set_var_value(result, assign, *lvalp << rval);
        }
        else {
            set_var_value(result, assign, *lvalp >> rval);
        }
    }
    else if( is_cmd(cmd, "NOT" ) ) {
        char *right = get_match(match, "RIGHT");
        c_signal *right_val = get_var(result, right);
        c_signal *val       = get_var_or_init(result, assign);

        if( DEBUG )
            fprintf(stderr, "NOT %s %d -> %s %d\n", right, *right_val, assign, *val);
        
        *val = ~*right_val;
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
