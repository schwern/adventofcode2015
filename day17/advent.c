#include "common.h"
#include <glib.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>

typedef int container_size_t;
typedef long target_size_t;
typedef long combo_size_t;

static inline bool is_in_combo( combo_size_t combo, int i ) {
    return combo & (1<<i);
}

static bool try_combo(GArray *containers, combo_size_t combo, target_size_t target) {
    size_t num_containers = containers->len;

    target_size_t sum = 0;
    for( int i = 0; i < num_containers; i++ ) {
        if( is_in_combo(combo, i) )
            sum += g_array_index(containers, container_size_t, i);

        if( sum > target )
            return false;
    }

    return sum == target;
}

static void test_try_combo() {
    printf("test_try_combo...");
    
    container_size_t test_data[] = {20, 15, 10, 5, 5};
    GArray *containers = g_array_new(false, true, sizeof(container_size_t));

    g_array_append_vals(containers, test_data, 5);

    g_assert_false( try_combo(containers, 0L, 25) );
    g_assert_false( try_combo(containers, 1L, 25) );
    g_assert_false( try_combo(containers,  ((1<<0) + (1<<3) + (1<<4)), 25) );    

    // 15 and 10
    g_assert_true( try_combo(containers,  ((1<<1) + (1<<2)), 25) );
    
    // 20 and the first 5
    g_assert_true( try_combo(containers,  ((1<<0) + (1<<3)), 25) );

    // 20 and the second 5
    g_assert_true( try_combo(containers,  ((1<<0) + (1<<4)), 25) );

    // 15, 5, and 5
    g_assert_true( try_combo(containers,  ((1<<1) + (1<<3) + (1<<4)), 25) );
    
    g_array_unref(containers);

    puts("OK");
}

static void read_container(char *line, void *_containers) {
    if( is_blank(line) )
        return;
    
    GArray *containers = (GArray*)_containers;

    errno = 0;
    container_size_t size = strtol(line, NULL, 10);
    if( size == 0 && errno == EINVAL)
        die("Unknown line '%s'", line);

    g_array_append_val(containers, size);

    return;
}

static void test_read_container() {
    printf("test_read_container...");
    
    GArray *containers = g_array_new(false, true, sizeof(container_size_t));

    read_container("", containers);
    g_assert_cmpuint( containers->len, ==, 0 );

    read_container("  ", containers);
    g_assert_cmpuint( containers->len, ==, 0 );

    read_container("23\n", containers);
    g_assert_cmpuint( containers->len, ==, 1 );
    g_assert_cmpint( g_array_index(containers, container_size_t, 0), ==, 23 );

    read_container("0\n", containers);
    g_assert_cmpuint( containers->len, ==, 2 );
    g_assert_cmpint( g_array_index(containers, container_size_t, 1), ==, 0 );
    
    g_array_unref(containers);

    puts("OK");
}

static GArray *read_containers(FILE *input) {
    GArray *containers = g_array_new(false, true, sizeof(container_size_t));

    foreach_line(input, read_container, containers);
    
    return containers;
}

static void print_combo(GArray *containers, combo_size_t combo, container_size_t num) {
    for( int i = 0; i < num; i++ ) {
        if( is_in_combo(combo, i) )
            printf("%d ", g_array_index(containers, container_size_t, i));
    }
    puts("");
}

static GArray *find_combos(GArray *containers, target_size_t target) {
    GArray *combos = g_array_new(false, true, sizeof(combo_size_t));
    size_t num_containers = containers->len;
    
    for( combo_size_t combo = 0; combo <= (1<<num_containers); combo++ ) {
        if( try_combo(containers, combo, target) )
            g_array_append_val(combos, combo);
    }
        
    return combos;
}

static void runtests() {
    test_try_combo();
    test_read_container();
}

int main(int argc, char *argv[]) {
    if( argc == 1 ) {
        runtests();
    }
    else if( argc == 3 ) {
        FILE *input = open_file(argv[1], "r");
        GArray *containers = read_containers(input);
        GArray *combos = find_combos(containers, atol(argv[2]));

        for( int i = 0; i < combos->len; i++ ) {
            print_combo(containers, g_array_index(combos, combo_size_t, i), containers->len);
        }
        
        g_array_unref(containers);
        g_array_unref(combos);
    }
    else {
        char *desc[] = {argv[0], "<input file>", "<storage target>"};
        usage(3, desc);
    }
    
    return 0;
}
