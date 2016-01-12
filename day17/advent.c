#include "common.h"
#include <glib.h>
#include <stdio.h>
#include <assert.h>

static bool try_combo(GArray *containers, long combo, int target) {
    size_t num_containers = containers->len;

    int sum = 0;
    for( int i = 0; i < num_containers; i++ ) {
        if( combo & (1<<i) )
            sum += g_array_index(containers, int, i);

        if( sum > target )
            return false;
    }

    return sum == target;
}

static void test_try_combo() {
    printf("test_try_combo...");
    
    int test_data[] = {20, 15, 10, 5, 5};
    GArray *containers = g_array_new(false, true, sizeof(int));

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

static void runtests() {
    test_try_combo();
}

int main(int argc, char *argv[]) {
    runtests();
    
    return 0;
}
