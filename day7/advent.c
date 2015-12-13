#include "common.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include "gate.h"

GRegex *Gate_Line_Re;
GRegex *Blank_Line_Re;

static void init_regexes() {
    if( !Blank_Line_Re ) {
        Blank_Line_Re = compile_regex(
            "^ \\s* $",
            G_REGEX_OPTIMIZE | G_REGEX_EXTENDED,
            0
        );
    }
    
    if( !Gate_Line_Re ) {
        Gate_Line_Re = compile_regex(
            " ^ \\s* "
            " (?: "
            "   (?:(?:(?<LEFT>[[:alnum:]]+) \\s+ )? (?<OP>[[:alpha:]]+) \\s+ )? (?<RIGHT>[[:alnum:]]+) "
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
    g_regex_unref(Gate_Line_Re);
}

static gint cmp_keys(gconstpointer _a, gconstpointer _b) {
    char *a = (char *)_a;
    char *b = (char *)_b;

    return strcmp(a, b);
}

static void print_gate_cb(gpointer key, gpointer val, gpointer user_data) {
    char *name = (char *)key;
    Gate *gate = (Gate *)val;
    GateOp *op = gate->proto->op;

    printf("Gate %s/%s %s", name, gate->name, op->name);
    if( op->type != UNDEF )
        printf(" = %d", __(gate, get));
    puts("");
}

static void gates_foreach_sorted(GHashTable *gates, GHFunc cb) {
    GList *sorted_keys = g_list_sort(g_hash_table_get_keys( gates ), cmp_keys);
    for (GList *k = sorted_keys; k != NULL; k = k->next) {
        cb( k->data, g_hash_table_lookup( gates, k->data ), NULL );
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

static Gate *read_gate_line(char *line, char **inputs) {
    GMatchInfo *match;

    if( g_regex_match(Blank_Line_Re, line, 0, NULL) ) {
        if( DEBUG )
            fprintf(stderr, "Blank line.\n");
        return NULL;
    }
    
    if( !g_regex_match(Gate_Line_Re, line, 0, &match) ) {
        fprintf(stderr, "Cannot understand %s.\n", line);
        return NULL;
    }
    
    char *name   = get_match(match, "NAME");
    char *opname = get_match(match, "OP");
    char *right  = get_match(match, "RIGHT");
    char *left   = get_match(match, "LEFT");
    if( is_empty(opname) ) {
        opname= "SET";
    }
    
    Gate *gate = Gate_factory(Op_lookup(opname), name);

    /* For "OP right", right is the first input */
    if( is_empty(left) ) {
        inputs[0] = strdup(right);
    }
    /* For "left OP right", left is the first input */
    else {
        inputs[0] = strdup(left);
        inputs[1] = strdup(right);
    }
    
    g_match_info_free(match);

    return gate;
}

static void set_gate_inputs(GHashTable *gates, Gate *gate, char **inputs) {
    GateOpType optype = gate->proto->op->type;

    /* Turn 123 -> a into a CONST op */
    if( optype == SET && is_number(inputs[0])) {
        __(gate, set_op, &Op_Const);
        __(gate, set_value, atoi(inputs[0]));
        return;
    }
        
    for( int i = 0; i < gate->proto->op->num_inputs; i++ ) {
        char *input_str = inputs[i];
        Gate *input_gate;

        if( !(input_gate = g_hash_table_lookup(gates, input_str)) ) {
            if( is_number(input_str) ) {
                input_gate = Gate_factory(&Op_Const, input_str);
                __(input_gate, set_value, atoi(input_str));
            }
            else {
                input_gate = Gate_factory(&Op_Undef, input_str);
            }

            g_hash_table_insert(gates, input_str, input_gate);
        }

        __(gate, set_input, i, input_gate);
    }
}

static GHashTable *read_circuit(FILE *fp) {
    char *line = NULL;
    size_t line_size = 0;
    char **inputs = calloc(2, sizeof(char));
    GHashTable *gates = g_hash_table_new(g_str_hash, g_str_equal);

    init_regexes();
    
    while( getline(&line, &line_size, fp) > 0 ) {
        Gate *gate = read_gate_line(line, inputs);
        if( !gate ) {
            printf("Unknown line: %s", line);
        }
        else {
            g_hash_table_insert(gates, gate->name, gate);
            set_gate_inputs(gates, gate, inputs);
        }
    }
    free(line);
    free(inputs);
    
    free_regexes();
    
    return gates;
}

int main(const int argc, char **argv) {
    FILE *input = stdin;

    if( argc > 2 ) {
        char *argv_desc[2] = {argv[0], "<circuit file>"};
        usage(2, argv_desc);
    }

    if( argc == 2 )
        input = open_file(argv[1], "r");

    GHashTable *gates = read_circuit(input);
    gates_foreach_sorted(gates, print_gate_cb);

    g_hash_table_unref(gates);

    return 0;
}
