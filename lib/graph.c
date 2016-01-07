#include "common.h"
#include <assert.h>
#include <stdlib.h>
#include <glib.h>
#include "graph.h"

Graph *Graph_new(GraphNodeNum max_nodes) {
    Graph *graph = malloc(sizeof(Graph));

    graph->node2name  = calloc(max_nodes, sizeof(*(graph->node2name)));
    graph->nodes      = calloc(max_nodes * max_nodes, sizeof(*(graph->nodes)));
    graph->name2node  = g_hash_table_new_full(
        g_str_hash, g_str_equal,
        free, free
    );

    graph->num_nodes = 0;
    graph->max_nodes = max_nodes;

    /* Set all node connections to infinite cost */
    for(GraphNodeNum x = 0; x < graph->num_nodes; x++) {
        for(GraphNodeNum y = 0; y < graph->num_nodes; y++) {
            Graph_add(graph, x, y, INFINITY);
        }
    }
    
    return graph;
}

void Graph_destroy(Graph *self) {
    free(self->nodes);

    /* This frees the names in all structures */
    g_hash_table_unref( self->name2node );

    free(self->node2name);
    
    free(self);
}

static inline GraphNodeSet GraphNodeSet_fill(GraphNodeNum size) {
    return (1<<size)-1;
}

/* Convert a node index to a bitmask on GraphNodeSet */
static inline GraphNodeSet GraphNodeSet_mask(GraphNodeNum x) {
    return 1 << x;
}

static inline bool GraphNodeSet_is_only_one_in_set(GraphNodeSet set, GraphNodeNum x) {
    return GraphNodeSet_mask(x) == set;
}

static inline bool GraphNodeSet_is_in_set(GraphNodeSet set, GraphNodeNum x) {
    return GraphNodeSet_mask(x) & set;
}

static inline GraphNodeSet GraphNodeSet_remove_from_set(GraphNodeSet set, GraphNodeNum x) {
    return ~GraphNodeSet_mask(x) & set;
}

static inline char *GraphNodeSet_to_human(GraphNodeSet set) {
    GraphNodeNum bits = sizeof(GraphNodeSet)*8;
    char *human = calloc(1, bits + 1);

    for( GraphNodeNum i = 0; i < bits; i++ ) {
        GraphNodeNum idx = bits - i - 1;
        human[idx] = GraphNodeSet_is_in_set(set, i) ? '1' : '0';
    }

    return human;
}

int Graph_min_cost_Calls = 0;
static GraphCost Graph_min_cost(Graph *self, GraphNodeNum start, GraphNodeNum current, GraphNodeSet visited) {
    if( DEBUG ) {
        char *human = GraphNodeSet_to_human(visited);
        fprintf(stderr, "min_cost(%p, %d, %d, %s)\n", self, start, current, human);
        free(human);
    }
    
    /* We could return 0, but it probably indicates an error in the algorithm */
    assert( start != current );

    /* We must have already visited the start and current nodes */
    assert( GraphNodeSet_is_in_set(visited, start) );
    assert( GraphNodeSet_is_in_set(visited, current) );
    
    /* Symmetrical optimization.
       That is, if you swap your start and end points, but visit the same places
       in-between, it's going to be the same minimum distance. */
    if( start > current ) {
        return Graph_min_cost( self, current, start, visited );
    }

    /* After the symmetric optimization */
    Graph_min_cost_Calls++;

    /* Remove ourselves from the visited set, we're going to ask how we got here. */
    visited = GraphNodeSet_remove_from_set(visited, current);

    /* Terminating case */
    if( GraphNodeSet_is_only_one_in_set(visited, start) )
        return Graph_edge_cost(self, start, current);
    
    /* Figure out what it would cost to come from each visited node */
    GraphCost cost = INFINITY;
    for( GraphNodeNum prev = 0; prev < self->num_nodes; prev++ ) {
        /* Can't have come from it if we didn't visit it. */
        if( !GraphNodeSet_is_in_set( visited, prev ) )
            continue;

        /* Can only visit the start if it's the only thing left and we took care of that */
        if( prev == start )
            continue;

        if( DEBUG )
            fprintf(stderr, "prev: %d, current: %d\n", prev, current);
        
        GraphCost prev_cost = Graph_edge_cost(self, prev, current);
        if( DEBUG )
            fprintf(stderr, "\tedge cost: %.0f\n", prev_cost);
        
        prev_cost += Graph_min_cost(self, start, prev, visited);

        if( DEBUG )
            fprintf(stderr, "\tprev_cost: %.0f\n", prev_cost);
        
        cost = MIN(prev_cost, cost);

        if( DEBUG )
            fprintf(stderr, "\tcost: %.0f\n", cost);
    }

    return cost;
}

GraphCost Graph_shortest_route_cost(Graph *self, bool return_to_start) {
    Graph_min_cost_Calls = 0;
    
    GraphCost cost = INFINITY;
    for(GraphNodeNum start = 0; start < self->num_nodes; start++) {
        if( DEBUG )
            fprintf(stderr, "starting from %d\n", start);

        GraphCost try_cost = Graph_shortest_route_cost_from(self, start, return_to_start);

        if( DEBUG )
            fprintf(stderr, "cost: %.0f, try_cost: %.0f\n", cost, try_cost);
        
        cost = MIN( cost, try_cost );
    }

    if( DEBUG )
        fprintf(stderr, "Graph_min_cost calls = %d\n", Graph_min_cost_Calls);
    
    return cost;
}

GraphCost Graph_shortest_route_cost_from(Graph *self, GraphNodeNum start, bool return_to_start) {
    GraphCost cost = INFINITY;

    for( GraphNodeNum end = 0; end < self->num_nodes; end++ ) {
        if( start == end )
            continue;
        
        GraphNodeSet visited = 0;
        visited = GraphNodeSet_fill(self->num_nodes);

        GraphCost new_cost = Graph_min_cost(self, start, end, visited);
        if( return_to_start )
            new_cost += Graph_edge_cost(self, end, start);
        
        cost = MIN( cost, new_cost );
    }

    return cost;
}

GraphNodeNum Graph_lookup(Graph *self, char *name) {
    GHashTable *name2node = self->name2node;
    GraphNodeNum *num = g_hash_table_lookup( name2node, name );

    if( DEBUG )
        fprintf(stderr, "Graph_lookup(%p, %s) == %d\n", self, name, *num);
    
    return *num;
}

GraphNodeNum Graph_lookup_or_add(Graph *self, char *name) {
    GHashTable *name2node = self->name2node;
    GraphNodeNum *num = g_hash_table_lookup( name2node, name );
    
    if( !num ) {
        num = malloc(sizeof(num));
        *num = self->num_nodes;

        char *name_dup = strdup(name);
        
        g_hash_table_insert( name2node, name_dup, num );

        self->node2name[*num] = name_dup;
        
        self->num_nodes++;
    }

    return *num;
}

void Graph_add(Graph *self, GraphNodeNum from, GraphNodeNum to, GraphCost cost) {
    GraphNodeNum num_nodes = self->num_nodes;
    GraphNodeNum max_nodes = self->max_nodes;
    
    if( from > max_nodes || to > max_nodes )
        die("%d is too big, the graph can only handle %d nodes", MAX(from, to), max_nodes);

    /* Edge costs are symetrical */
    EDGE(self, from, to) = cost;
    EDGE(self, to, from) = cost;

    /* Increase the number of nodes, if necessary */
    if( from > num_nodes || to > num_nodes )
        self->num_nodes = MAX(from, to);
}

void Graph_add_named(Graph *self, char *from, char *to, GraphCost cost) {
    GraphNodeNum from_num = Graph_lookup_or_add(self, from);
    GraphNodeNum to_num   = Graph_lookup_or_add(self, to);

    Graph_add(self, from_num, to_num, cost);
}

GraphCost Graph_edge_cost_named(Graph *self, char *from, char *to) {
    GraphNodeNum from_num = Graph_lookup(self, from);
    GraphNodeNum to_num   = Graph_lookup(self, to);

    return Graph_edge_cost(self, from_num, to_num);
}

void Graph_increment(Graph *self, GraphNodeNum from, GraphNodeNum to, GraphCost cost) {
    EDGE(self, from, to) = (EDGE(self, from, to) + cost);
}

void Graph_increment_named(Graph *self, char *from, char *to, GraphCost cost) {
    GraphNodeNum from_num = Graph_lookup(self, from);
    GraphNodeNum to_num   = Graph_lookup(self, to);

    return Graph_increment(self, from_num, to_num, cost);
}

void Graph_print(Graph *self) {
    for(GraphNodeNum x = 0; x < self->num_nodes; x++) {
        for(GraphNodeNum y = 0; y < self->num_nodes; y++) {
            GraphCost cost = Graph_edge_cost(self, x, y);

            if( cost ) {
                char *x_name = self->node2name[x];
                char *y_name = self->node2name[y];

                printf("%s/%d to %s/%d = %.0f\n", x_name, x, y_name, y, cost);
            }
        }
    }
}
