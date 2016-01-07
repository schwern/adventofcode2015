#include "common.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "graph.h"

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
            "^ \\s* "
            "  (?<FROM>[[:alpha:]]+) \\s+ to \\s+ (?<TO>[[:alpha:]]+) \\s* = \\s* (?<COST>\\d+) "
            "\\s* $ ",
            G_REGEX_OPTIMIZE | G_REGEX_EXTENDED,
            0
        );
}

static void free_regexes() {
    g_regex_unref(Blank_Line_Re);
    g_regex_unref(Line_Re);
}

static void read_node(char *line, void *_graph) {
    Graph *graph = (Graph *)_graph;
    GMatchInfo *match;
    
    if( g_regex_match(Line_Re, line, 0, &match) ) {
        char *from              = g_match_info_fetch_named(match, "FROM");
        char *to                = g_match_info_fetch_named(match, "TO");
        char *cost_str          = g_match_info_fetch_named(match, "COST");
        GraphCost cost = (GraphCost)atoi(cost_str);
        Graph_add_named(graph, from, to, cost);
        
        g_match_info_free(match);
        free(from);
        free(to);
        free(cost_str);
    }
    else if( g_regex_match(Blank_Line_Re, line, 0, NULL) ) {
        return;
    }
    else {
        die("Unknown line '%s'", line);
    }
}

static Graph *read_graph(FILE *input) {
    Graph *graph = Graph_new(20);

    init_regexes();

    foreach_line(input, read_node, graph);

    free_regexes();
    
    return graph;
}

int main(int argc, char **argv) {
    FILE *input = stdin;

    if( argc >= 2 ) {
        input = open_file(argv[1], "r");
    }

    Graph *graph = read_graph(input);

    printf("%.0f\n", Graph_shortest_route_cost(graph, false));
    
    if( DEBUG )
        Graph_print(graph);
    
    Graph_destroy(graph);
    
    return 0;
}
