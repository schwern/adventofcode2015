#include "common.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/* http://adventofcode.com/day/10 */

/* We only need to store 0 to 3. */
typedef uint8_t SequenceNum;

typedef struct {
    SequenceNum *seq;
    size_t size;
} Sequence;

static Sequence *Sequence_new(size_t size) {
    Sequence *seq = malloc(sizeof(Sequence));

    seq->size   = size;
    seq->seq    = calloc(size, sizeof(seq->seq));

    return seq;
}

static void Sequence_destroy(Sequence *seq) {
    free(seq->seq);
    free(seq);
}

static Sequence *Sequence_new_from_string(char *string) {
    size_t len = strlen(string);
    Sequence *seq = Sequence_new(len);

    for(int i = 0; i < len; i++) {
        seq->seq[i] = string[i] - '0';
    }

    return seq;
}

static char *Sequence_to_string(Sequence *seq) {
    char *string = malloc(seq->size + 1);
    string[seq->size] = '\0';
    
    for(int i = 0; i < seq->size; i++) {
        string[i] = '0' + seq->seq[i];
    }
    
    return string;
}

static size_t Sequence_next_size(Sequence *seq) {
    /* 1 becomes 11, 2 becomes 12... */
    if( seq->size == 1 )
        return 2;
    
    int changes = 0;

    SequenceNum prev = 0;
    for(size_t i = 0; i < seq->size; i++) {
        if( prev != seq->seq[i] )
            changes++;

        prev = seq->seq[i];
    }

    return changes * 2;
}

static Sequence *look_and_say_once(Sequence *seq) {
    Sequence *next_seq = Sequence_new( Sequence_next_size(seq) );

    /* 1 -> 11, 2 -> 12, 3 -> 13 */
    if( seq->size == 1 ) {
        next_seq->seq[0] = 1;
        next_seq->seq[1] = seq->seq[0];

        return next_seq;
    }

    int next_pos = 0;
    SequenceNum prev = seq->seq[0];
    for(size_t i = 0; i < seq->size; i++) {
        if( prev != seq->seq[i] ) {
            next_pos += 2;
            prev = seq->seq[i];
        }
        
        next_seq->seq[next_pos]++;
        next_seq->seq[next_pos+1] = prev;
    }

    return next_seq;
}

static Sequence *look_and_say(char *seq_str, int times) {
    Sequence *seq = Sequence_new_from_string(seq_str);
    Sequence *next_seq = seq;
    
    for(int i = 1; i <= times; i++) {
        next_seq = look_and_say_once(seq);
        Sequence_destroy(seq);
        seq = next_seq;
    }

    return next_seq;
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
        "13112221",
        "1113213211",
        "31131211131221",
        ""
    };

    printf("Sequence_new_from_string / to_string round trip\n");
    for(int i = 0; !is_empty(results[i]); i++) {
        char *arg = results[i];
        char *have = Sequence_to_string( Sequence_new_from_string(arg) );
        char *want = arg;
        assert( streq( have, want ) );
    }

    printf("Sequence_next_size\n");
    for(int i = 0; !is_empty(results[i+1]); i++) {
        char *arg = results[i];
        int want = strlen(results[i+1]);
        int have = Sequence_next_size( Sequence_new_from_string(arg) );

        printf("\tSequence_next_size(%s), have: %d, want: %d\n", arg, have, want);
        assert( have == want );
    }
    
    
    printf("Trying look_and_say(x, 0)\n");
    for(int i = 0; !is_empty(results[i]); i++) {
        char *arg  = results[i];
        char *want = arg;
        char *have = Sequence_to_string( look_and_say(arg, 0) );
        
        printf("\tlook_and_say(%s, 0), have: %s, want: %s\n", arg, have, want);
        assert( streq(have, want) );
    }
    
    printf("Trying look_and_say(x, 1)\n");
    for(int i = 0; !is_empty(results[i+1]); i++) {
        char *arg  = results[i];
        char *want = results[i+1];
        char *have = Sequence_to_string( look_and_say(arg, 1) );
        
        printf("\tlook_and_say(%s, 1), have: %s, want: %s\n", arg, have, want);
        assert( streq(have, want) );
    }

    printf("Trying look_and_say_len(x, y)\n");
    for( int i = 0; !is_empty(results[i]); i++ ) {
        char *arg  = results[0];
        char *want = results[i];
        char *have = Sequence_to_string( look_and_say(arg, i) );
        
        printf("\tlook_and_say(%s, %d), have: %s, want: %s\n", arg, i, have, want);
        assert( streq(have, want) );
    }
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
        printf( "Trying %s %s times.\n", argv[1], argv[2] );
        Sequence *result = look_and_say(argv[1], atoi(argv[2]));
        printf("Length is %zu\n", result->size);
        Sequence_destroy(result);
    }
    else {
        puts("Running tests.");
        run_tests();
    }

    return 0;
}
