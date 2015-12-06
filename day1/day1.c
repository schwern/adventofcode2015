#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

static int read_floor_instructions(FILE *fp) {
    char c;
    int floor = 0;
    
    while( !feof(fp) ) {
        c = (char)fgetc(fp);
        switch(c) {
            case '(':
                floor++;
                break;
            case ')':
                floor--;
                break;
        }
    }

    return floor;
}

int main(int argc, char **argv) {
    /* Check the arguments */
    if( argc != 2 ) {
        fprintf(stderr, "Usage: %s <inputfile>\n", argv[0]);
        return -1;
    }

    /* Open the floor file */
    FILE *floor_fp = fopen(argv[1], "r");
    if( floor_fp == NULL ) {
        fprintf(stderr, "Could not open %s: %s.\n", argv[1], strerror(errno));
        return errno;
    }
    
    int floor = read_floor_instructions(floor_fp);
    printf("The instructions take Santa to floor %d.\n", floor);

    return 0;
}
