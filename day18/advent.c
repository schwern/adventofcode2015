#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <glib.h>
#include <stdbool.h>
#include <string.h>

typedef bool Light;
typedef struct {
    size_t max_rows;
    size_t max_cols;
    size_t rows;
    Light *grid;
} Lights;

static Lights *Lights_new(size_t rows, size_t cols) {
    Lights *lights = calloc(1, sizeof(Lights));

    lights->max_rows = rows;
    lights->max_cols = cols;
    lights->grid = calloc(rows*cols, sizeof(Light));
    
    lights->rows = 0;

    return lights;
}

static Lights *Lights_new_from_array(size_t rows, size_t cols, Light *grid) {
    Lights *lights = calloc(1, sizeof(Lights));

    lights->max_rows = rows;
    lights->max_cols = cols;
    lights->rows = rows;

    lights->grid = grid;

    return lights;
}

static Lights *Lights_new_from_array_copy(size_t rows, size_t cols, Light *orig_grid) {
    Light *copy_grid = calloc(rows*cols, sizeof(Light));
    memcpy(copy_grid, orig_grid, rows * cols * sizeof(Light));
    return Lights_new_from_array(rows, cols, copy_grid);
}

static void Lights_destroy(Lights *self) {
    free(self->grid);
    free(self);
}

static inline void Lights_set(Lights *self, size_t row, size_t col, Light setting) {
    assert(row < self->max_rows);
    assert(col < self->max_cols);
    TWOD(self->grid, row, col, self->max_rows) = setting;
}

static inline Light Lights_get(Lights *self, size_t row, size_t col) {
    return TWOD(self->grid, row, col, self->max_rows);
}

static inline size_t Lights_get_next_row(Lights *self) {
    return self->rows;
}

static inline void Lights_print(Lights *self) {
    for( int row = 0; row < self->rows; row++ ) {
        for( int col = 0; col < self->max_cols; col++ ) {
            if( Lights_get(self, row, col) ) {
                printf("#");
            }
            else {
                printf(".");
            }
        }

        puts("");
    }
}

static void assert_lights_eq(Lights *have, Lights *want) {
    g_assert_cmpuint(have->rows, ==, want->rows);
    g_assert_cmpuint(have->max_cols, ==, want->max_cols);

    for( int row = 0; row < want->max_rows; row++ ) {
        for( int col = 0; col < want->max_cols; col++ ) {
            g_assert_cmpint( Lights_get(have, row, col), ==, Lights_get(want, row, col) );
        }
    }
}

static void read_light_line(char *line, void *_lights) {
    Lights *lights = (Lights *)_lights;
    size_t row = Lights_get_next_row(lights);
    lights->rows++;
    
    for( int col = 0; line[col] != '\0'; col++ ) {
        char c = line[col];
        switch(c) {
            case '.':
                Lights_set(lights, row, col, false);
                break;
            case '#':
                Lights_set(lights, row, col, true);
                break;
            case '\n':
                break;
            default:
                die("Unknown char '%c' in line '%s'", c, line);
                break;
        }
    }
}

static void test_read_lights() {
    printf("test_read_lights...");
    
    char *input[] = {
        ".#.#.#",
        "...##.",
        "#....#",
        "..#...",
        "#.#..#",
        "####..",
        NULL
    };

    Light want_array[6*6] = {
        false, true, false, true, false, true,
        false, false, false, true, true, false,
        true, false, false, false, false, true,
        false, false, true, false, false, false,
        true, false, true, false, false, true,
        true, true, true, true, false, false
    };
    Lights *want = Lights_new_from_array_copy(6, 6, want_array);
    
    Lights *have = Lights_new(6, 6);
    for( int i = 0; input[i] != NULL; i++ ) {
        read_light_line(input[i], have);
    }

    if( DEBUG ) {
        puts("Have");
        Lights_print(have);
        puts("Want");
        Lights_print(want);
    }
    
    assert_lights_eq(have, want);

    Lights_destroy(want);
    Lights_destroy(have);

    puts("OK");
}

static void runtests() {
    test_read_lights();
}

int main(int argc, char *argv[]) {
    runtests();
    
    return 0;
}
