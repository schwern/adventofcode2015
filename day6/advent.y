%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "advent.y.h"
    #include "advent.l.h"
    #include "common.h"

    #define MAX_LIGHTS 1000

    int *Lights;

    void change_lights(int *lights, const char *command, int where[2][2]);
    void yyerror (YYLTYPE *loc, char const *msg);
%}

%defines
%locations
%define api.pure full
%define parse.error verbose
                                                
%union {
    char *command;
    int num;
    int coord[2];
}

%token <num> NUMBER
%token <command> COMMAND
%token END COMMA THROUGH
%left ','

%type   <coord> coordinate

%%

lines : line |
lines line

line: exp END |
exp;

coordinate: NUMBER COMMA NUMBER  {
    $$[0] = $1; $$[1] = $3;
}

exp: COMMAND coordinate THROUGH coordinate {
    int coords[2][2] = {{$2[0], $2[1]}, {$4[0], $4[1]}};
    change_lights(Lights, $1, coords);
    free($1);
}


%%

void yyerror (YYLTYPE *loc, char const *msg) {
    fprintf(stderr, "%s at %d:%d - %d:%d\n",
            msg,
            loc->first_column, loc->first_line,
            loc->last_column, loc->last_line
    );
}

static inline int light_key(int row, int col) {
    return row + col * MAX_LIGHTS;
}

static void dim_lights(int *lights, int where[2][2], int dim) {
    int *from = where[0];
    int *to   = where[1]; 
    
    int row;
    int col;

    for( row = from[0]; row <= to[0]; row++ ) {
        for( col = from[1]; col <= to[1]; col++ ) {
            int key = light_key(row, col);
            lights[key] += dim;
            if( lights[key] < 0 )
                lights[key] = 0;
        }
    }
}

void change_lights(int *lights, const char *command, int where[2][2]) {
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

static int light_brightness(int *lights) {
    int brightness = 0;
    
    for(int row = 0; row < MAX_LIGHTS; row++) {
        for(int col = 0; col < MAX_LIGHTS; col++) {
            int key = light_key(row, col);
            brightness += lights[key];
        }
    }

    return brightness;
}

static void init_lights(int **lights) {
    *lights = calloc(1, sizeof(int) * MAX_LIGHTS * MAX_LIGHTS);
}

static void free_lights(int *lights) {
    free(lights);
}

int main(int argc, char **argv) {
    FILE *input = stdin;

    if( argc > 2 ) {
        char *desc[2] = {argv[0], "<light file>"};
        usage(2, desc);
        return 1;
    }
    
    if( argv[1] ) {
        input = open_file(argv[1], "r");
    }

    init_lights(&Lights);
    
    yyin = input;
    do {
        yyparse();
    } while(!feof(yyin));

    printf("%d\n", light_brightness(Lights));

    free_lights(Lights);
}
