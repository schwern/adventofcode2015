#ifndef _graph_h
#define _graph_h

#include "common.h"
#include <stdint.h>
#include <glib.h>
#include <math.h>

/* Because we store distances as unsigned ints to save memory, we
   can't represent infinity for no connection.  0 or MAX_INT are our
   choices.  0 is a problem when comparing, MAX_INT is a problem when
   adding.  So use floating point infinity when calculating
   distances */
typedef float GraphCost;
typedef short GraphDistance;

/* Let's face it, this isn't going to work for even 255 nodes */
typedef uint8_t GraphNodeNum;

/* XXX This isn't big enough XXX */
typedef int GraphNodeSet;

typedef struct {
    GHashTable *name2node;
    char **node2name;
    GraphDistance *nodes;
    GraphNodeNum max_nodes;
    GraphNodeNum num_nodes;
} Graph;

Graph *Graph_new(GraphNodeNum max_nodes);
void Graph_destroy(Graph *self);
int Graph_shortest_route_cost(Graph *self, bool return_to_start);
int Graph_shortest_route_cost_from(Graph *self, GraphNodeNum start, bool return_to_start);
void Graph_add_named(Graph *self, char *from, char *to, GraphDistance distance);
void Graph_increment_named(Graph *self, char *from, char *to, GraphDistance distance);
void Graph_increment(Graph *self, GraphNodeNum from, GraphNodeNum to, GraphDistance distance);
void Graph_add_or_increment_named(Graph *self, char *from, char *to, GraphDistance distance);
GraphNodeNum Graph_lookup_or_add(Graph *self, char *name);
void Graph_print(Graph *self);

#define EDGE(graph, x, y) TWOD(graph->nodes, x, y, graph->max_nodes)
static inline GraphCost Graph_edge_cost(Graph *self, GraphNodeNum x, GraphNodeNum y) {
    GraphCost cost = EDGE(self, x, y);
    return cost == 0 ? INFINITY : cost;
}

GraphCost Graph_edge_cost_named(Graph *self, char *from, char *to);

#endif
