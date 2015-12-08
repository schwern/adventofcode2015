%{
    #include <stdio.h>
    #include "advent.y.h"
    #include "advent.l.h"

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

line: exp END;

coordinate: NUMBER COMMA NUMBER  {
    $$[0] = $1; $$[1] = $3;
}

exp: COMMAND coordinate THROUGH coordinate {
    printf("%s: %d,%d %d,%d\n", $1, $2[0], $2[1], $4[0], $4[1]);
}


%%

void yyerror (YYLTYPE *loc, char const *msg) {
    fprintf(stderr, "%s at %d:%d - %d:%d\n",
            msg,
            loc->first_column, loc->first_line,
            loc->last_column, loc->last_line
    );
}


int main() {
    do {
        yyparse();
    } while(!feof(yyin));
}
