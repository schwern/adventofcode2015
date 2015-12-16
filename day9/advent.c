#include "common.h"
#include "glib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

typedef struct {
    GHashTable *name2node;
    char **node2name;
    short *nodes;
    short max_nodes;
    short num_nodes;
} Graph;


static Graph *Graph_new(short max_nodes) {
    Graph *graph = malloc(sizeof(Graph));

    graph->node2name  = calloc(max_nodes, sizeof(*(graph->node2name)));
    graph->nodes      = calloc(max_nodes * max_nodes, sizeof(*(graph->node2name)));
    graph->name2node  = g_hash_table_new_full(
        g_str_hash, g_str_equal,
        free, free
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

#define EDGE(graph, x, y) TWOD(graph->nodes, x, y, graph->max_nodes)

/* Because we store distances as unsigned shorts to save memory, we
   can't represent infinity for no connection.  0 or MAX_INT are our
   choices.  0 is a problem when comparing, MAX_INT is a problem when
   adding.  So use floating point infinity when calculating
   distances */
static inline float Graph_edge_cost(Graph *self, short x, short y) {
    float cost = EDGE(self, x, y);
    return cost == 0 ? INFINITY : cost;
}

typedef short GraphNodeSet;

static inline GraphNodeSet GraphNodeSet_fill(short size) {
    return (1<<size)-1;
}

/* Convert a node index to a bitmask on GraphNodeSet */
static inline short GraphNodeSet_mask(short x) {
    return 1 << x;
}

static inline bool GraphNodeSet_is_only_one_in_set(GraphNodeSet set, short x) {
    return GraphNodeSet_mask(x) == set;
}

static inline bool GraphNodeSet_is_in_set(GraphNodeSet set, short x) {
    return GraphNodeSet_mask(x) & set;
}

static inline GraphNodeSet GraphNodeSet_remove_from_set(GraphNodeSet set, short x) {
    return ~GraphNodeSet_mask(x) & set;
}

static inline char *GraphNodeSet_to_human(GraphNodeSet set) {
    short bits = sizeof(GraphNodeSet)*8;
    char *human = calloc(1, bits + 1);

    for( short i = 0; i < bits; i++ ) {
        short idx = bits - i - 1;
        human[idx] = GraphNodeSet_is_in_set(set, i) ? '1' : '0';
    }

    return human;
}

static inline GraphNodeSet GraphNodeSet_flip(GraphNodeSet set, short x, short y) {
    return set ^ (GraphNodeSet_mask(x) | GraphNodeSet_mask(y));
}

static float Graph_min_cost(Graph *self, short start, short next, GraphNodeSet visited) {
    if( DEBUG ) {
        char *human = GraphNodeSet_to_human(visited);
        fprintf(stderr, "min_cost(%p, %d, %d, %s)\n", self, start, next, human);
        free(human);
    }
    
    /* We could return 0, but it probably indicates an error in the algorithm */
    assert( start != next );

    /* We must not have already visited the next node */
    assert( !GraphNodeSet_is_in_set(visited, next) );
    
    /* Symmetrical optimization.
           min_cost(x, y, visited) == min_cost(y, x, visited ^ (x|y))
       That is, if you swap your start and end points, but visit the same places
       in-between, it's going to be the same minimum distance. */
    if( start > next ) {
        return Graph_min_cost( self, next, start, GraphNodeSet_flip(visited, start, next) );
    }

    /* Figure out what it would cost to come from each visited node */
    float cost = INFINITY;
    for( short prev = 0; prev < self->num_nodes; prev++ ) {
        if( prev == start ) {
            /* Only the starting point has been visited, next must be one hop away */
            if( GraphNodeSet_is_only_one_in_set(visited, start) ) {
                float edge_cost = Graph_edge_cost(self, start, next);
                if( DEBUG )
                    fprintf(stderr, "Starting edge %d to %d = %f\n", start, next, edge_cost);
                return edge_cost;
            }
            /* We can't have come from the start if others have been visited */
            else
                continue;
        }

        if( DEBUG )
            fprintf(stderr, "cost = %f\n", cost);
        
        /* This node was previously visited */
        if( GraphNodeSet_is_in_set(visited, prev) ) {
            /* The edge cost to get from prev to next */
            float prev_cost = Graph_edge_cost(self, next, prev);

            if( DEBUG )
                fprintf(stderr, "prev edge cost = %f\n", prev_cost);
            
            /* No point in continuing, it's more expensive than another route */
            if( prev_cost > cost )
                continue;

            /* The cost to back to the start from prev */
            prev_cost += Graph_min_cost(self, start, prev, GraphNodeSet_remove_from_set(visited, prev));

            if( DEBUG )
                fprintf(stderr, "prev_cost = %f\n", prev_cost);
            
            /* Is this the best yet? */
            cost = MIN(cost, prev_cost);
        }
    }

    return cost;
}

static short Graph_shortest_route(Graph *self) {
    float cost = INFINITY;
    for(short start = 0; start < self->num_nodes; start++) {
        for( short end = start+1; end < self->num_nodes; end++) {
            GraphNodeSet visited = 0;
            visited = GraphNodeSet_fill(self->num_nodes);
            visited = GraphNodeSet_remove_from_set(visited, end);
            if( DEBUG )
                fprintf(stderr, "Trying start to end\n");
            cost = MIN( cost, Graph_min_cost(self, start, end, visited) );
        }
    }

    return cost;
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

    /* Edge costs are symetrical */
    EDGE(self, from, to) = distance;
    EDGE(self, to, from) = distance;

    /* Increase the number of nodes, if necessary */
    if( from > num_nodes || to > num_nodes )
        self->num_nodes = MAX(from, to);
}

static void Graph_add_named(Graph *self, char *from, char *to, short distance) {
    short from_num = Graph_lookup_or_add(self, from);
    short to_num   = Graph_lookup_or_add(self, to);

    Graph_add(self, from_num, to_num, distance);
}

static void Graph_print(Graph *self) {
    for(short x = 0; x < self->num_nodes; x++) {
        for(short y = 0; y < self->num_nodes; y++) {
            short distance = EDGE(self, x, y);
            float cost     = Graph_edge_cost(self, x, y);

            if( distance ) {
                char *x_name = self->node2name[x];
                char *y_name = self->node2name[y];

                printf("%s/%d to %s/%d = %d/%f\n", x_name, x, y_name, y, distance, cost);
            }
        }
    }
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

    if( DEBUG )
        Graph_print(graph);

    printf("%d\n", Graph_shortest_route(graph));
    
    Graph_destroy(graph);
    
    return 0;
}
