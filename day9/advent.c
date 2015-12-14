#include "common.h"
#include "glib.h"
#include <stdio.h>

void print_line(char *line, void *_count) {
    int *count = (int *)_count;
    
    printf("%s", line);
    (*count)++;
}

int find_shortest_route(FILE *input) {
    int count = 0;
    foreach_line(input, print_line, &count);
    return count;
}

int main(int argc, char **argv) {
    FILE *input = stdin;

    if( argc >= 2 ) {
        input = open_file(argv[1], "r");
    }

    int shortest = find_shortest_route(input);

    printf("%d\n", shortest);

    return 0;
}
