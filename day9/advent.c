#include "common.h"
#include "glib.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    GHashTable *name2node;
    char **node2name;
    short *nodes;
    short max_nodes;
    short num_nodes;
} Graph;

static void Graph_key_destroy(gpointer key) {
    free(key);
}

static Graph *Graph_new(int max_nodes) {
    Graph *graph = malloc(sizeof(Graph));

    graph->node2name  = calloc(max_nodes, sizeof(*(graph->node2name)));
    graph->nodes      = calloc(max_nodes * max_nodes, sizeof(*(graph->node2name)));
    graph->name2node  = g_hash_table_new_full(
        g_str_hash, g_str_equal,
        Graph_key_destroy, Graph_key_destroy
    );

    graph->num_nodes = 0;
    graph->max_nodes = max_nodes;
    
    return graph;
}

static void Graph_destroy(Graph *self) {
    free(self->nodes);

    /* This frees the names in all structures */
    g_hash_table_unref( self->name2node );

    free(self->node2name);
    
    free(self);
}

static short Graph_lookup_or_add(Graph *self, char *name) {
    GHashTable *name2node = self->name2node;
    short *num = g_hash_table_lookup( name2node, name );
    
    if( !num ) {
        num = malloc(sizeof(short));
        *num = self->num_nodes;

        char *name_dup = strdup(name);
        
        g_hash_table_insert( name2node, name_dup, num );

        self->node2name[*num] = name_dup;
        
        self->num_nodes++;
    }

    return *num;
}

static void Graph_add(Graph *self, short from, short to, short distance) {
    short num_nodes = self->num_nodes;
    short max_nodes = self->max_nodes;
    
    if( from > max_nodes || to > max_nodes )
        die("%d is too big, the graph can only handle %d nodes", MAX(from, to), max_nodes);

    TWOD(self->nodes, from, to) = distance;

    /* Increase the number of nodes, if necessary */
    if( from > num_nodes || to > num_nodes )
        self->num_nodes = MAX(from, to);
}

static void Graph_add_named(Graph *self, char *from, char *to, short distance) {
    short from_num = Graph_lookup_or_add(self, from);
    short to_num   = Graph_lookup_or_add(self, to);

    Graph_add(self, from_num, to_num, distance);
}


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
            "  (?<FROM>[[:alpha:]]+) \\s+ to \\s+ (?<TO>[[:alpha:]]+) \\s* = \\s* (?<DISTANCE>\\d+) "
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
        char *distance_str      = g_match_info_fetch_named(match, "DISTANCE");
        short distance = (short)atoi(distance_str);
        fprintf(stderr, "From %s to %s = %d\n", from, to, distance);
        Graph_add_named(graph, from, to, distance);
        
        g_match_info_free(match);
        free(from);
        free(to);
        free(distance_str);
    }
    else if( g_regex_match(Blank_Line_Re, line, 0, NULL) ) {
        return;
    }
    else {
        die("Unknown line '%s'", line);
    }
}

static Graph *read_graph(FILE *input) {
    Graph *graph = Graph_new(100);

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

    printf("%d\n", graph->num_nodes);

    for( int i = 0; i < graph->num_nodes; i++ ) {
        printf("%d -> %s\n", i, graph->node2name[i]);
    }
    
    Graph_destroy(graph);
    
    return 0;
}
