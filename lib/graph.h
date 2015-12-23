#ifndef _graph_h
#define _graph_h

/* Because we store distances as unsigned ints to save memory, we
   can't represent infinity for no connection.  0 or MAX_INT are our
   choices.  0 is a problem when comparing, MAX_INT is a problem when
   adding.  So use floating point infinity when calculating
   distances */
typedef float GraphCost;
typedef unsigned short GraphDistance;

/* Let's face it, this isn't going to work for even 255 nodes */
typedef uint8_t GraphNodeNum;

typedef struct {
    GHashTable *name2node;
    char **node2name;
    GraphDistance *nodes;
    GraphNodeNum max_nodes;
    GraphNodeNum num_nodes;
} Graph;

Graph *Graph_new(GraphNodeNum max_nodes);
void Graph_destroy(Graph *self);
int Graph_shortest_route_cost(Graph *self);
void Graph_add_named(Graph *self, char *from, char *to, GraphDistance distance);
void Graph_print(Graph *self);

#endif
