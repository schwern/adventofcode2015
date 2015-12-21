#include "common.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

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

/* Passwords must be... */
static bool is_valid_password(char *password) {
    size_t len = strlen(password);
    
    /* ...exactly eight lowercase letters */
    if( len != 8 )
        return false;

    short straight_cnt          = 0;
    short max_straight_cnt      = 0;
    short pair_cnt              = 0;
    bool  prev_was_pair = false;

    char prev_char = password[0];
    for( int i = 1; i < len; i++ ) {
        char this_char = password[i];
        
        /* may not contain the letters i, o, or l */
        switch(this_char) {
            case 'i':
            case 'o':
            case 'l':
                return false;
                break;
        }

        if( prev_char == this_char - 1 ) {
            straight_cnt += 2;
            max_straight_cnt = MAX( straight_cnt, max_straight_cnt );
        }
        else {
            straight_cnt = 0;
        }

        if( !prev_was_pair && prev_char == this_char ) {
            pair_cnt++;
            prev_was_pair = true;
        }
        else {
            prev_was_pair = false;
        }
        
        prev_char = this_char;
    }

    if( DEBUG )
        fprintf(stderr, "max_straight_cnt: %d, pair_cnt: %d\n", max_straight_cnt, pair_cnt);
    
    /* must include one increasing straight of at least three letters */
    if( max_straight_cnt < 3 )
        return false;

    /* must contain at least two different, non-overlapping pairs of letters */
    if( pair_cnt < 2 )
        return false;
    
    return true;
}


static void test_is_valid_password() {
    char *valid[] = {
        "abcdffaa",
        "ghjaabcc",
        ""
    };

    printf("Testing is_valid_password()\n");
    for(int i = 0; !is_empty(valid[i]); i++) {
        char *arg = valid[i];

        printf("\tis_valid_password(%s)\n", arg);
        assert( is_valid_password(arg) );
    }
    
    char *invalid[] = {
        "abc",          /* too short */
        "ghjaabccg",    /* too long */
        "hijklmmn",     /* invalid char i */
        "abbceffg",     /* missing straight */
        "abbcegjk",     /* only one pair */
        "abbbcdef",     /* a triple is not two pairs */
        ""
    };

    printf("Testing !is_valid_password()\n");
    for(int i = 0; !is_empty(invalid[i]); i++) {
        char *arg = invalid[i];

        printf("\t!is_valid_password(%s)\n", arg);
        assert( !is_valid_password(arg) );
    }
}


static char *next_password(char *old) {
    char *new = strdup(old);
    
    do {
        inc_string(new);
    } while( !is_valid_password(new) );

    return new;
}


static void test_next_password() {
    char *tests[] = {
        "abcdefgh", "abcdffaa",
        "ghijklmn", "ghjaabcc",
        ""
    };

    printf("Testing next_password()\n");
    for( int i = 0; !is_empty(tests[i]); i+=2 ) {
        char *arg = tests[i];
        char *want = tests[i+1];

        char *have = next_password(arg);

        printf("\tnext_password(%s) == %s/%s\n", arg, have, want);
        assert( streq(have, want) );

        free(have);
    }
}


static void tests() {
    test_inc_string();
    test_is_valid_password();
    test_next_password();
}


int main(int argc, char **argv) {
    if( argc == 1 ) {
        tests();
    }
    else if( argc == 2 ) {
        char *new = next_password( argv[1] );

        puts(new);
        free(new);
    }
    else {
        char *desc[] = {argv[0], "<old password>"};
        usage(2, desc);
    }
}
