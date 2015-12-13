#include "common.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include "gate.h"

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
            "   (?<VAL>[[:alnum:]]+) | "
            "   (?:(?<LEFT>[[:alnum:]]+) \\s+)? (?<CMD>[[:alpha:]]+) \\s+ (?<RIGHT>[[:alnum:]]+) "
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

static gint cmp_keys(gconstpointer _a, gconstpointer _b) {
    char *a = (char *)_a;
    char *b = (char *)_b;

    return strcmp(a, b);
}

static void print_state_cb(gpointer _key, gpointer _val, gpointer user_data) {
    char *key    = (char *)_key;
    GateVal val = *(GateVal *)_val;

    printf("%s: %d\n", key, val);
}

static void state_foreach_sorted(GHashTable *state, GHFunc cb) {
    GList *sorted_keys = g_list_sort(g_hash_table_get_keys( state ), cmp_keys);
    for (GList *k = sorted_keys; k != NULL; k = k->next) {
        cb( k->data, g_hash_table_lookup( state, k->data ), NULL );
    }
}

static inline GateVal *init_val() {
    GateVal *val = malloc(sizeof(GateVal));
    *val = 0;
    
    return val;
}

static inline char *get_match(GMatchInfo *match, char *key) {
    char *val = g_match_info_fetch_named(match, key);
    if( !val ) {
        fprintf(stderr, "%s not found in %s\n", key, g_match_info_get_string(match));
        exit(1);
    }

    return val;
}

static inline GateVal *get_var(GHashTable *state, char *key) {
    GateVal *val = g_hash_table_lookup(state, key);
    if( !val ) {
        val = init_val();
        g_hash_table_insert(state, key, val);
    }

    return val;
}

static bool is_number(char *key) {
    /* Empty string */
    if( key == '\0' )
        return false;
    
    for( ; key[0] != '\0'; key++ ) {
        if( !g_ascii_isdigit(key[0]) )
            return false;
    }

    return true;
}

static inline GateVal *get_val(GHashTable *state, char *key) {
    if( is_number(key) ) {
        GateVal *val = init_val();
        *val = atoi(key);
        return val;
    }
    else {
        return get_var(state, key);
    }
}

static inline void set_var(GHashTable *state, char *key, GateVal *val) {
    g_hash_table_replace(state, key, val);
}

static inline void set_var_value(GHashTable *state, char *key, GateVal _val) {
    GateVal *val = init_val();
    *val = _val;
    set_var(state, key, val);
}

static inline bool is_cmd(char *have, char *want) {
    return strcmp(g_ascii_strup(have, -1), g_ascii_strup(want, -1)) == 0 ? true : false;
}

static void process_circuit_line(GHashTable *state, char *line) {
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
        GateVal *valp = get_val( state, get_match(match, "VAL") );
        if( DEBUG )
            fprintf(stderr, "%d -> %s\n", *valp, assign);
        set_var(state, assign, valp);
    }
    else if(
        is_cmd(cmd, "AND") || is_cmd(cmd, "OR") ||
        is_cmd(cmd, "LSHIFT") || is_cmd(cmd, "RSHIFT")
    ) {
        char *left  = get_match(match, "LEFT");
        char *right = get_match(match, "RIGHT");
        GateVal *lvalp = get_val(state, left);
        GateVal *rvalp = get_val(state, right);

        if( DEBUG )
            fprintf(stderr, "%s %d %s %s %d -> %s\n", left, *lvalp, cmd, right, *rvalp, assign);
        
        if( is_cmd(cmd, "AND") )
            set_var_value(state, assign, *lvalp & *rvalp);
        else if( is_cmd(cmd, "OR" ) )
            set_var_value(state, assign, *lvalp | *rvalp);
        else if( is_cmd(cmd, "LSHIFT") )
            set_var_value(state, assign, *lvalp << *rvalp);
        else
            set_var_value(state, assign, *lvalp >> *rvalp);
    }
    else if( is_cmd(cmd, "NOT" ) ) {
        char *right = get_match(match, "RIGHT");
        GateVal *rvalp = get_var(state, right);
        GateVal *avalp = get_var(state, assign);

        if( DEBUG )
            fprintf(stderr, "NOT %s %d -> %s %d\n", right, *rvalp, assign, *avalp);
        
        *avalp = ~*rvalp;
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
    GHashTable *state = g_hash_table_new(g_str_hash, g_str_equal);

    init_regexes();
    
    while( getline(&line, &line_size, fp) > 0 ) {
        process_circuit_line(state, line);
    }

    free_regexes();
    
    return state;
}

int main(const int argc, char **argv) {
    FILE *input = stdin;

    if( argc > 2 ) {
        char *argv_desc[2] = {argv[0], "<circuit file>"};
        usage(2, argv_desc);
    }

    if( argc == 2 )
        input = open_file(argv[1], "r");

    GHashTable *state = read_circuit(input);
    state_foreach_sorted(state, print_state_cb);
    g_hash_table_unref(state);

    return 0;
}
