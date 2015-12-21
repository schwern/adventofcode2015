#include "common.h"
#include <stdio.h>
#include <assert.h>

static void inc_string(char *string) {
    size_t len = strlen(string);

    static char First_Char = 'a';
    static char Last_Char  = 'z';
    
    for( int i = len-1; i >= 0; i-- ) {
        if( string[i] != Last_Char ) {
            string[i]++;
            break;
        }
        else {
            string[i] = First_Char;
        }
    }
}

static void test_inc_string() {
    char *tests[] = {
        "foo", "fop",
        "xz", "ya",
        "yzzz", "zaaa",
        ""
    };

    printf("Testing inc_string()\n");
    for(int i = 0; !is_empty(tests[i]); i += 2) {
        char *arg  = tests[i];
        char *have = strdup(tests[i]);
        const char *want = tests[i+1];

        inc_string(have);

        printf("\tinc_string(%s) == %s / %s\n", arg, have, want);
        assert( streq(have, want) );
        
        free(have);
    }
}


static char *next_password(char *old) {
    return "";
}


static void tests() {
    test_inc_string();
}


int main(int argc, char **argv) {
    if( argc == 1 ) {
        tests();
    }
    else if( argc == 2 ) {
        char *new = next_password( argv[0] );

        puts(new);
        free(new);
    }
    else {
        char *desc[] = {argv[0], "<old password>"};
        usage(2, desc);
    }
}
