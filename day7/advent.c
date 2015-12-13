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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

static gint cmp_keys(gconstpointer _a, gconstpointer _b) {
    char *a = (char *)_a;
    char *b = (char *)_b;

    return strcmp(a, b);
}

static void print_gate_cb(gpointer key, gpointer val, gpointer user_data) {
    Gate *gate = (Gate *)val;
    GateOp *op = gate->proto->op;

    if( op->num_inputs == 0 ) {
        printf("%s %s %d", gate->name, op->name, __(gate, get));
    }
    if( op->num_inputs == 1 ) {
        printf("%s %s %s", gate->name, op->name, gate->inputs[0]->name);
    }
    else if( op->num_inputs == 2 ) {
        printf("%s %s %s %s", gate->name, gate->inputs[0]->name, op->name, gate->inputs[1]->name);
    }
    puts("");
}

static void reset_gate_cache(gpointer key, gpointer val, gpointer unused) {
    Gate *gate = (Gate *)val;

    __(gate, clear_cache);
}

static void gates_foreach_sorted(GHashTable *gates, GHFunc cb) {
    GList *sorted_keys = g_list_sort(g_hash_table_get_keys( gates ), cmp_keys);
    for (GList *k = sorted_keys; k != NULL; k = k->next) {
        cb( k->data, g_hash_table_lookup( gates, k->data ), NULL );
    }
}

#pragma clang diagnostic pop

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
        /* Allow opname to be consistenty freed */
        opname = realloc(opname, 4);
        strlcpy(opname, "SET", 4);
    }
    
    Gate *gate = Gate_factory(Op_lookup(opname), name);

    /* For "OP right", right is the first input */
    if( is_empty(left) ) {
        inputs[0] = right;
        /* Return left so inputs can be consistently freed */
        inputs[1] = left;
    }
    /* For "left OP right", left is the first input */
    else {
        inputs[0] = left;
        inputs[1] = right;
    }
    
    g_match_info_free(match);
    free(name);
    free(opname);
    
    return gate;
}

static Gate *make_input_gate(GHashTable *gates, char *var) {
    Gate *gate = g_hash_table_lookup(gates, var);
    if( gate )
        return gate;
    
    if( is_number(var) ) {
        gate = Gate_factory(&Op_Const, var);
        __(gate, set_cache, atoi(var));
    }
    else {
        gate = Gate_factory(&Op_Undef, var);
    }

    g_hash_table_insert(gates, gate->name, gate);

    return gate;
}

static inline void change_gate_to_const(Gate *gate, GateVal val) {
    __(gate, set_op, &Op_Const);
    __(gate, set_cache, val);
}

static void set_gate_inputs(GHashTable *gates, Gate *gate, char **inputs) {
    GateOpType optype = gate->proto->op->type;

    /* Turn 123 -> a into a CONST op */
    if( optype == SET && is_number(inputs[0])) {
        change_gate_to_const(gate, atoi(inputs[0]));
        return;
    }
        
    for( int i = 0; i < gate->proto->op->num_inputs; i++ ) {
        char *input_str = inputs[i];
        Gate *input_gate = make_input_gate(gates, input_str);

        __(gate, set_input, i, input_gate);
    }
}

static Gate *check_gate_cache(GHashTable *gates, Gate *gate) {
    Gate *cached_gate = g_hash_table_lookup(gates, gate->name);

    /* It's not cached, cache it */
    if( !cached_gate ) {
        g_hash_table_insert(gates, gate->name, gate);
        return gate;
    }

    if( cached_gate->proto->op->type != UNDEF ) {
        printf("Redefining gate %s\n", gate->name);
    }
    
    __(cached_gate, set_op, gate->proto->op);
    __(gate, destroy);

    return cached_gate;
}

static GHashTable *read_circuit(FILE *fp) {
    char *line = NULL;
    size_t line_size = 0;
    char **inputs = calloc(2, sizeof(char *));
    GHashTable *gates = g_hash_table_new(
        g_str_hash,
        g_str_equal
    );

    init_regexes();
    
    while( getline(&line, &line_size, fp) > 0 ) {
        Gate *gate = read_gate_line(line, inputs);
        if( !gate ) {
            printf("Unknown line: %s", line);
        }
        else {
            gate = check_gate_cache(gates, gate);
            set_gate_inputs(gates, gate, inputs);

            free(inputs[0]);
            free(inputs[1]);
        }
    }
    free(line);
    free(inputs);
    
    free_regexes();
    
    return gates;
}

int main(const int argc, char **argv) {
    FILE *input = stdin;

    if( argc > 4 ) {
        char *argv_desc[4] = {argv[0], "<circuit file>", "<var> <override>"};
        usage(4, argv_desc);
    }

    if( argc >= 2 )
        input = open_file(argv[1], "r");

    GHashTable *gates = read_circuit(input);
    //gates_foreach_sorted(gates, print_gate_cb);

    if( argc >= 3 ) {
        char *var = argv[2];
        Gate *gate = g_hash_table_lookup(gates, var);
        GateVal signal = Gate_get(gate);
        printf("%s == %d\n", var, signal);

        if( argc >= 4 ) {
            char *override_var = argv[3];
            Gate *override = g_hash_table_lookup(gates, override_var);
            g_hash_table_foreach(gates, reset_gate_cache, 0);
            change_gate_to_const(override, signal);
            printf("Overrode %s with %d\n", override_var, signal);
            printf("%s == %d\n", var, Gate_get(gate));
        }
    }

    g_hash_table_destroy(gates);

    return 0;
}
