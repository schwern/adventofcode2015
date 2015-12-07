#include <stdio.h>
#include <stdlib.h>
#include "file.h"

struct Floors {
    int start_floor;
    int end_floor;
    int first_enter_basement;
};

static struct Floors *Floors_create() {
    struct Floors *floors = malloc(sizeof(struct Floors));

    floors->start_floor = 0;
    floors->end_floor = 0;
    floors->first_enter_basement = -1;

    return floors;
}

static struct Floors *read_floor_instructions(FILE *fp) {
    char c;
    int position = 0;
    struct Floors *floors = Floors_create();
    
    while( !feof(fp) ) {
        c = (char)fgetc(fp);
        switch(c) {
            case '(':
                position++;
                floors->end_floor++;
                break;
            case ')':
                position++;
                floors->end_floor--;
                break;
        }

        if( floors->first_enter_basement < 0 && floors->end_floor < 0 ) {
            floors->first_enter_basement = position;
        }
    }

    return floors;
}

int main(int argc, char **argv) {
    char *argv_desc[2] = { argv[0], "<inputfile>" };
    if( !usage(argc, 2, argv_desc) ) {
        return -1;
    }

    FILE *floor_fp = open_file(argv[1], "r");
    
    struct Floors *floors = read_floor_instructions(floor_fp);
    printf("The instructions take Santa to floor %d.\n", floors->end_floor);
    if( floors->first_enter_basement > 0 ) {
        printf("Santa enters the basement at position %d.\n", floors->first_enter_basement);
    }

    free(floors);
    
    return 0;
}
