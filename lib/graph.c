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

static inline GraphNodeSet GraphNodeSet_flip(GraphNodeSet set, GraphNodeNum x, GraphNodeNum y) {
    return set ^ (GraphNodeSet_mask(x) | GraphNodeSet_mask(y));
}

int Graph_min_cost_Calls = 0;
static GraphCost Graph_min_cost(Graph *self, GraphNodeNum start, GraphNodeNum next, GraphNodeSet visited) {
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

    /* After the symmetric optimization */
    Graph_min_cost_Calls++;
    
    /* Figure out what it would cost to come from each visited node */
    GraphCost cost = INFINITY;
    for( GraphNodeNum prev = 0; prev < self->num_nodes; prev++ ) {
        if( prev == start ) {
            /* Only the starting point has been visited, next must be one hop away */
            if( GraphNodeSet_is_only_one_in_set(visited, start) ) {
                GraphCost edge_cost = Graph_edge_cost(self, start, next);
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
            GraphCost prev_cost = Graph_edge_cost(self, next, prev);

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

int Graph_shortest_route_cost(Graph *self, bool return_to_start) {
    Graph_min_cost_Calls = 0;
    
    GraphCost cost = INFINITY;
    for(GraphNodeNum start = 0; start < self->num_nodes; start++) {
        cost = MIN( cost, Graph_shortest_route_cost_from(self, start, return_to_start ) );
    }

    if( DEBUG )
        fprintf(stderr, "Graph_min_cost calls = %d\n", Graph_min_cost_Calls);
    
    return cost;
}

int Graph_shortest_route_cost_from(Graph *self, GraphNodeNum start, bool return_to_start) {
    GraphCost cost = INFINITY;

    for( GraphNodeNum end = 0; end < self->num_nodes; end++ ) {
        if( start == end )
            continue;
        
        GraphNodeSet visited = 0;
        visited = GraphNodeSet_fill(self->num_nodes);
        visited = GraphNodeSet_remove_from_set(visited, end);

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

static void Graph_add(Graph *self, GraphNodeNum from, GraphNodeNum to, GraphDistance distance) {
    GraphNodeNum num_nodes = self->num_nodes;
    GraphNodeNum max_nodes = self->max_nodes;
    
    if( from > max_nodes || to > max_nodes )
        die("%d is too big, the graph can only handle %d nodes", MAX(from, to), max_nodes);

    /* Edge costs are symetrical */
    EDGE(self, from, to) = distance;
    EDGE(self, to, from) = distance;

    /* Increase the number of nodes, if necessary */
    if( from > num_nodes || to > num_nodes )
        self->num_nodes = MAX(from, to);
}

void Graph_add_named(Graph *self, char *from, char *to, GraphDistance distance) {
    GraphNodeNum from_num = Graph_lookup_or_add(self, from);
    GraphNodeNum to_num   = Graph_lookup_or_add(self, to);

    Graph_add(self, from_num, to_num, distance);
}

GraphCost Graph_edge_cost_named(Graph *self, char *from, char *to) {
    GraphNodeNum from_num = Graph_lookup(self, from);
    GraphNodeNum to_num   = Graph_lookup(self, to);

    return Graph_edge_cost(self, from_num, to_num);
}

void Graph_increment(Graph *self, GraphNodeNum from, GraphNodeNum to, GraphDistance distance) {
    EDGE(self, from, to) = (EDGE(self, from, to) + distance);
}

void Graph_increment_named(Graph *self, char *from, char *to, GraphDistance distance) {
    GraphNodeNum from_num = Graph_lookup(self, from);
    GraphNodeNum to_num   = Graph_lookup(self, to);

    return Graph_increment(self, from_num, to_num, distance);
}

void Graph_print(Graph *self) {
    for(GraphNodeNum x = 0; x < self->num_nodes; x++) {
        for(GraphNodeNum y = 0; y < self->num_nodes; y++) {
            GraphDistance distance = EDGE(self, x, y);
            GraphCost cost         = Graph_edge_cost(self, x, y);

            if( distance ) {
                char *x_name = self->node2name[x];
                char *y_name = self->node2name[y];

                printf("%s/%d to %s/%d = %d/%f\n", x_name, x, y_name, y, distance, cost);
            }
        }
    }
}
