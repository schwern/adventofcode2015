#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "file.h"

typedef struct {
    int paper;
    int ribbon;
} Order;

static Order *Order_create() {
    Order *order = malloc(sizeof(Order));

    order->paper  = 0;
    order->ribbon = 0;

    return order;
}

typedef struct {
    int sides[3];
} Box;

#define HEIGHT 0
#define WIDTH  1
#define LENGTH 2

static inline int min(const int x, const int y) {
    return x < y ? x : y;
}

static Box *Box_create(const int *dimensions) {
    Box *box = malloc(sizeof(Box));

    for(int i = 0; i < 3; i++) {
        box->sides[i] = dimensions[i];
    }

    return box;
}

static int Box_volume(const Box *box) {
    return box->sides[0] * box->sides[1] * box->sides[2];
}

static int cmpsides(const void *arg1, const void *arg2) {
    const int a = *(const int*)arg1;
    const int b = *(const int*)arg2;

    if( a < b ) {
        return -1;
    }
    else if( a > b ) {
        return 1;
    }
    else {
        return 0;
    }

    assert(0);
}

static int Box_smallest_perimeter(const Box *box) {
    int sides[3];

    for(int i = 0; i < 3; i++) {
        sides[i] = box->sides[i];
    }

    qsort(sides, 3, sizeof(*sides), cmpsides);

    return sides[0] + sides[0] + sides[1] + sides[1];
}

static int Box_ribbon(const Box *box) {
    return Box_smallest_perimeter(box) + Box_volume(box);
}

static int Box_surface_area(const Box *box) {
    return (2 * box->sides[LENGTH] * box->sides[WIDTH])  +
           (2 * box->sides[WIDTH]  * box->sides[HEIGHT]) +
           (2 * box->sides[HEIGHT] * box->sides[LENGTH]);
}

static int Box_wrapping_paper_slack(const Box *box) {
    return min(box->sides[LENGTH] * box->sides[WIDTH],
               min(box->sides[WIDTH] * box->sides[HEIGHT],
                   box->sides[HEIGHT] * box->sides[LENGTH]
               )
           );
}

static int Box_wrapping_paper(const Box *box) {
    return Box_surface_area(box) + Box_wrapping_paper_slack(box);
}

static Box *parse_box_line(const char *orig_line) {
    /* Copy the line, because strsep() is destructive
     * strsep() will move line around, so hold onto a pointer to the
     * original memory so we can free it */
    char *tofree, *line;
    tofree = line = strdup(orig_line);
    assert(line != NULL);

    int dimensions[3];
    int idx = 0;
    char *token;
    while((token = strsep(&line, "x")) != NULL) {
        dimensions[idx] = atoi(token);
        idx++;
    }

    free(tofree);
    
    return Box_create(dimensions);
}

static Order *read_box_sizes(FILE *fp) {
    char *line = NULL;
    size_t linecap = 0;
    Box *box;
    Order *order = Order_create();
    
    while(getline(&line, &linecap, fp) > 0) {
        box = parse_box_line(line);
        order->paper  += Box_wrapping_paper(box);
        order->ribbon += Box_ribbon(box);
        free(box);
    }
    free(line);

    return order;
}

int main(const int argc, char **argv) {
    if( argc != 2 ) {
        char *desc[2] = {argv[0], "<inputfile>"};
        usage(2, desc);
        return -1;
    }

    FILE *fp = open_file(argv[1], "r");

    Order *order = read_box_sizes(fp);
    printf("The elves need %d sqft of paper and %d ft of ribbon.\n", order->paper, order->ribbon);

    free(order);
    
    return 0;
}
