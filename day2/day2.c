#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/errno.h>

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

    for(int i = 0; i <= 2; i++) {
        box->sides[i] = dimensions[i];
    }

    return box;
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
    char *line = strdup(orig_line);
    assert(line != NULL);

    int dimensions[3];
    int idx = 0;
    char *token;
    while((token = strsep(&line, "x")) != NULL) {
        dimensions[idx] = atoi(token);
        idx++;
    }
    free(line);

    return Box_create(dimensions);
}

static long read_box_sizes(FILE *fp) {
    char *line;
    size_t linecap = 0;
    Box *box;
    int paper = 0;
    
    while(getline(&line, &linecap, fp) > 0) {
        box = parse_box_line(line);
        paper += Box_wrapping_paper(box);
    }

    return paper;
}

int main(int argc, char **argv) {
    /* Check the arguments */
    if( argc != 2 ) {
        fprintf(stderr, "Usage: %s <inputfile>\n", argv[0]);
        return -1;
    }

    /* Open the file */
    FILE *fp = fopen(argv[1], "r");
    if( fp == NULL ) {
        fprintf(stderr, "Could not open %s: %s.\n", argv[1], strerror(errno));
        return errno;
    }

    long paper = read_box_sizes(fp);
    printf("The elves need %ld square feet of paper.\n", paper);

    return 0;
}
