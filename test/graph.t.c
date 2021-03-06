#include "graph.h"
#include <assert.h>
#include <stdio.h>

void test_increment() {
    Graph *graph = Graph_new(20);

    Graph_lookup_or_add(graph, "Foo");
    Graph_lookup_or_add(graph, "Bar");

    Graph_add_named(graph, "Foo", "Bar", 20);
    Graph_increment_named(graph, "Foo", "Bar", -5);

    assert( Graph_edge_cost_named(graph, "Foo", "Bar") == 15 );
}

void test_lookup_or_add() {
    Graph *graph = Graph_new(20);

    GraphNodeNum foo_num = Graph_lookup_or_add(graph, "Foo");
    GraphNodeNum bar_num = Graph_lookup_or_add(graph, "Bar");

    assert( foo_num == Graph_lookup_or_add(graph, "Foo") );
    assert( foo_num != bar_num );
}

int main(int argc, char **argv) {
    test_lookup_or_add();
    test_increment();
    printf("%s: PASS\n", argv[0]);
}
