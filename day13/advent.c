#include "common.h"
#include "graph.h"
#include <glib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

GRegex *Line_Re;
GRegex *Blank_Line_Re;

static void init_regexes() {
    if( !Blank_Line_Re )
        Blank_Line_Re = compile_regex(
            "^ \\s* $",
            G_REGEX_OPTIMIZE | G_REGEX_EXTENDED,
            0
        );

    if( !Line_Re )
        Line_Re = compile_regex(
            "^\\s*"
            "(?<FROM>[[:alpha:]]+) would (?<SIGN>gain|lose) (?<HAPPINESS>\\d+) happiness units by sitting next to (?<TO>[[:alpha:]]+)\\."
            "\\s*$",
            G_REGEX_OPTIMIZE,
            0
        );
}

static void free_regexes() {
    g_regex_unref(Blank_Line_Re);
    g_regex_unref(Line_Re);
}

void read_node( char *line, void *_graph ) {
    Graph *graph = (Graph *)_graph;
    GMatchInfo *match;

    if( g_regex_match(Line_Re, line, 0, &match) ) {
        char *from      = g_match_info_fetch_named(match, "FROM");
        char *to        = g_match_info_fetch_named(match, "TO");
        char *happiness = g_match_info_fetch_named(match, "HAPPINESS");
        char *sign      = g_match_info_fetch_named(match, "SIGN");
        GraphCost cost = (GraphCost)atoi(happiness);

        if( streq(sign, "gain") )
            cost = -cost;

        GraphNodeNum from_num = Graph_lookup_or_add(graph, from);
        GraphNodeNum to_num   = Graph_lookup_or_add(graph, to);
        
        /* We only care about the total happiness gained/lost.
           It's easier if we model this as a symmetrical cost */
        Graph_increment(graph, from_num, to_num, cost);
        Graph_increment(graph, to_num, from_num, cost);

        g_match_info_free(match);
        free(from);
        free(to);
        free(happiness);
        free(sign);
    }
    else if( g_regex_match(Blank_Line_Re, line, 0, NULL) ) {
        return;
    }
    else {
        die("Unknown line '%s'", line);
    }
    
    return;
}

void test_read_node() {
    Graph *graph = Graph_new(20);

    init_regexes();
    read_node( "Alice would gain 54 happiness units by sitting next to Bob.\n", graph );
    read_node( "Bob would lose 14 happiness units by sitting next to Alice.\n", graph );
    free_regexes();

    GraphNodeNum alice_num = Graph_lookup_or_add(graph, "Alice");
    GraphNodeNum bob_num   = Graph_lookup_or_add(graph, "Bob");

    GraphCost have = Graph_edge_cost( graph, alice_num, bob_num );
    GraphCost want = -40;
    printf("Graph_edge_cost( %p, %d, %d ) == %.0f/%.0f\n", graph, alice_num, bob_num, have, want);
    assert( have == want );

    have = Graph_edge_cost( graph, bob_num, alice_num );
    want = -40;
    printf("Graph_edge_cost( %p, %d, %d ) == %.0f/%.0f\n", graph, bob_num, alice_num, have, want);
    assert( have == want );
}

Graph *read_graph(FILE *input) {
    Graph *graph = Graph_new(30);

    init_regexes();
    foreach_line(input, read_node, graph);
    free_regexes();
    
    return graph;
}

void runtests() {
    test_read_node();
}

/* For part 2, add myself with 0 happiness change for everyone */
static void add_me(Graph *graph) {
    GraphNodeNum my_num = Graph_lookup_or_add(graph, "Me");

    for(GraphNodeNum x = 0; x < graph->num_nodes; x++) {
        if( x == my_num )
            continue;
        
        Graph_add(graph, my_num, x, 0);
        Graph_add(graph, x, my_num, 0);
    }
}

int main(int argc, char **argv) {
    if( argc == 1 ) {
        runtests();
    }
    else if( argc == 2 ) {
        FILE *input = open_file(argv[1], "r");
        Graph *graph = read_graph(input);

        add_me(graph);
        
        if( DEBUG )
            Graph_print(graph);
        
        int happiness = -Graph_shortest_route_cost_from(graph, 0, true);

        printf("%d\n", happiness);
    }
    else {
        char *desc[] = {argv[0], "<input file>"};
        usage(2, desc);
    }
}
