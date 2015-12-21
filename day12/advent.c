#include "common.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

static inline int ipow(int base, int power) {
    return pow(base, power);
}

static int sum_all_numbers( const char *string ) {
    size_t len = strlen(string);

    int sum = 0;
    int num = 0;
    int tens = 0;
    for( int i = len - 1; i >= 0; i-- ) {
        switch(string[i]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                num += ipow(10, tens) * (string[i] - '0');
                tens++;
                break;
            case '-':
                num = -num;
                break;
            default:
                sum += num;
                num = 0;
                tens = 0;
                break;
        }
    }

    sum += num;
    
    return sum;
}

static void test_sum_all_numbers() {
    char *tests[] = {
        "[1,2,3]",              "6",
        "{\"a\":2,\"b\":4}",    "6",
        "[[[3]]]",              "3",
        "{\"a\":{\"b\":4},\"c\":-1}",   "3",
        "{\"a\":[-1,1]}",       "0",
        "[-1,{\"a\":1}]",       "0",
        "[]",                   "0",
        "{}",                   "0",
        "2",                    "2",
        "[12, 102, 40]",        "154",
        ""
    };

    for(int i = 0; !is_empty(tests[i]); i += 2) {
        char *arg = tests[i];
        int want = atoi(tests[i+1]);

        int have = sum_all_numbers(arg);

        printf("\tsum_all_numbers(%s) == %d/%d\n", arg, have, want);
        assert( have == want );
    }
}

static void tests() {
    test_sum_all_numbers();
}

static int read_all_numbers(FILE *input) {
    return 42;
}

int main(int argc, char **argv) {
    if( argc == 1 ) {
        tests();
    }
    else if( argc == 2 ) {
        FILE *input = open_file(argv[1], "r");
        int sum = read_all_numbers(input);
        printf("%d\n", sum);
    }
    else {
        char *desc[] = {argv[0], "<input file>"};
        usage(2, desc);
    }
}
