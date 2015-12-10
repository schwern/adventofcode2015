#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "common.h"
#include <glib.h>

#define MAX_LIGHTS 1000

static void dim_lights(int16_t lights[MAX_LIGHTS][MAX_LIGHTS], int where[2][2], int dim) {
    int *from = where[0];
    int *to   = where[1]; 
    
    int row;
    int col;

    for( row = from[0]; row <= to[0]; row++ ) {
        for( col = from[1]; col <= to[1]; col++ ) {
            lights[row][col] += dim;
            if( lights[row][col] < 0 )
                lights[row][col] = 0;
        }
    }
}

static void change_lights(int16_t lights[MAX_LIGHTS][MAX_LIGHTS], const char *command, int where[2][2]) {
    if( strcmp(command, "turn on") == 0 ) {
        dim_lights(lights, where, +1);
    }
    else if( strcmp(command, "turn off") == 0 ) {
        dim_lights(lights, where, -1);
    }
    else if( strcmp(command, "toggle") == 0 ) {
        dim_lights(lights, where, +2);
    }
    else {
        fprintf(stderr, "Unknown command: %s\n", command);
    }
}

static int light_brightness(int16_t lights[MAX_LIGHTS][MAX_LIGHTS]) {
    int brightness = 0;
    
    for(int row = 0; row < MAX_LIGHTS; row++) {
        for(int col = 0; col < MAX_LIGHTS; col++) {
            brightness += lights[row][col];
        }
    }

    return brightness;
}

static void read_lights(FILE *fp, int16_t lights[MAX_LIGHTS][MAX_LIGHTS]) {
    char *line = NULL;
    size_t linelen = 0;

    GMatchInfo *match = NULL;
    GError *re_error = NULL;
    GRegex *re = g_regex_new(
        "^(?<CMD>[a-zA-Z ]+) (?<X1>\\d+),(?<Y1>\\d+) through (?<X2>\\d+),(?<Y2>\\d+)$",
        G_REGEX_CASELESS|G_REGEX_OPTIMIZE,
        0,
        &re_error
    );

    if( !re ) {
        fprintf(stderr, "Could not compile regex: %s.\n", re_error->message);
        exit(1);
    }

    while( getline(&line, &linelen, fp) > 0 ) {
        int where[2][2];

        if( !g_regex_match(re, line, 0, &match) ) {
            fprintf(stderr, "Could not parse '%s'\n", line);
        }

        gchar *command = g_match_info_fetch_named(match, "CMD");
        where[0][0] = atoi(g_match_info_fetch_named(match, "X1"));
        where[0][1] = atoi(g_match_info_fetch_named(match, "Y1"));
        where[1][0] = atoi(g_match_info_fetch_named(match, "X2"));
        where[1][1] = atoi(g_match_info_fetch_named(match, "Y2"));

        change_lights(lights, command, where);

        g_free(command);
    }

    g_match_info_free(match);
    g_regex_unref(re);
}

int main(int argc, char **argv) {
    FILE *input = stdin;
    int16_t lights[MAX_LIGHTS][MAX_LIGHTS] = {{0}};
    
    if( argv[1] ) {
        input = open_file(argv[1], "r");
    }

    read_lights(input, lights);
    printf("%d\n", light_brightness(lights));

    return 0;
}
