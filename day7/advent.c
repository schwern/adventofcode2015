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
            "   (?:(?<LEFT>[[:alnum:]]+) \\s+)? (?<OP>[[:alpha:]]+) \\s+ (?<RIGHT>[[:alnum:]]+) "
            " ) "
            " \\s* -> \\s* "
            " (?<NAME>[[:alpha:]]+) "
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

static inline char *get_match(GMatchInfo *match, char *key) {
    char *val = g_match_info_fetch_named(match, key);
    if( !val ) {
        fprintf(stderr, "%s not found in %s\n", key, g_match_info_get_string(match));
        exit(1);
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
    
    char *name = get_match(match, "NAME");
    char *opname = g_match_info_fetch_named(match, "OP");
    if( !opname || opname[0] == '\0' )
        opname = "CONST";
    
    if( DEBUG )
        fprintf(stderr, "Op: '%s'.\n", opname );

    Gate *gate = Gate_factory(Op_lookup(opname), name);

    printf("Gate %s with op %s\n", gate->name, gate->proto->op->name);
    
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
