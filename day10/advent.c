#include "common.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

/* http://adventofcode.com/day/10 */

static char *look_and_say_once(const char *initial_sequence) {
    const char *pos = initial_sequence;
    char letter[2];
    
    /* The maximum possible size should be twice the original, plus null */
    size_t result_len = (strlen(pos) * 2) + 1;
    char *result = calloc(result_len, sizeof(char));

    if( DEBUG )
        fprintf(stderr, "Result len = %zu\n", result_len);
    
    while( !is_empty(pos) ) {
        letter[0] = pos[0];
        letter[1] = '\0';

        if( DEBUG )
            fprintf(stderr, "Letter = %s\n", letter);
        
        size_t next_pos = strspn(pos, letter);

        char *num_str = num_to_str(next_pos);
        
        if( DEBUG )
            fprintf(stderr, "num_to_str(%zu) = %s\n", next_pos, num_str);

        strlcat( result, num_str, result_len );
        strlcat( result, letter, result_len );

        free(num_str);
        
        pos += next_pos;
    }

    return result;
}

static char *look_and_say(char *sequence, int times) {
    char *result = sequence;
    
    for( int i = 1; i <= times; i++ ) {
        sequence = result;
        result = look_and_say_once(sequence);

        /* Free the intermediate result, but not the initial one */
        if( i > 1 )
            free(sequence);
    }

    return result;
}


/* Test the example from the problem description */
static void test_example() {
    char *results[] = {
        "1",
        "11",
        "21",
        "1211",
        "111221",
        "312211",
        "\0"
    };

    printf("Trying look_and_say_once()\n");
    for(int i = 0; !is_empty(results[i+1]); i++) {
        char *arg  = results[i];
        char *want = results[i+1];

        char *have = look_and_say_once(results[i]);
        
        printf(
            "\targ:  %s, have: %s, want: %s\n",
            arg, have, want
        );
        assert( streq( have, want ) );
    }

    assert( streq( look_and_say(results[0], 5), results[5] ) );
}

static void run_tests() {
    test_example();
}


int main(int argc, char **argv) {
    if( argc > 3 ) {
        char *desc[3] = { argv[0], "<initial sequence>", "<num times>" };
        usage(3, desc);
        exit(1);
    }
    
    if( argc == 3 ) {
        printf( "Trying %s %d times.\n", argv[1], atoi(argv[2]) );
        char *result = look_and_say(argv[1], atoi(argv[2]));
        printf("Length is %zu\n", strlen(result));

        free(result);
    }
    else {
        puts("Running tests.");
        run_tests();
    }

    return 0;
}
