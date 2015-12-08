%{
    #include "advent.y.h"
%}

%option bison-bridge bison-locations
%option warn
%option noyywrap nounput noinput never-interactive
%option header-file="advent.l.h"

%%

[0-9]+ {
    yylval->num = atoi(yytext);
    return NUMBER;
}

"turn off" |
"turn on"  |
toggle {
    yylval->command = strdup(yytext);
    return COMMAND;
}

","  { return COMMA; }

"through" { return THROUGH; }
    
"\n" { return END; }

.
    
%%

