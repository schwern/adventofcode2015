#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

typedef short Light;

enum {
    OFF, ON, STUCK
};

typedef struct {
    size_t max_rows;
    size_t max_cols;
    size_t rows;
    Light *grid;
} Lights;

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

static Light *Lights_new_grid(Lights *self) {
    return calloc(self->max_rows * self->max_cols, sizeof(Light));
}

static Lights *Lights_new(size_t rows, size_t cols) {
    Lights *self = calloc(1, sizeof(Lights));

    self->max_rows = rows;
    self->max_cols = cols;
    self->grid = Lights_new_grid(self);
    
    self->rows = 0;

    return self;
}

static Lights *Lights_new_from_array(size_t rows, size_t cols, Light *grid) {
    Lights *self = calloc(1, sizeof(Lights));

    self->max_rows = rows;
    self->max_cols = cols;
    self->rows = rows;

    self->grid = grid;

    return self;
}

static Lights *Lights_new_from_array_copy(size_t rows, size_t cols, Light *orig_grid) {
    Light *copy_grid = calloc(rows*cols, sizeof(Light));
    memcpy(copy_grid, orig_grid, rows * cols * sizeof(Light));
    return Lights_new_from_array(rows, cols, copy_grid);
}

static void Lights_read_line(char *line, void *_lights) {
    Lights *lights = (Lights *)_lights;
    size_t row = Lights_get_next_row(lights);
    lights->rows++;
    
    for( int col = 0; line[col] != '\0'; col++ ) {
        char c = line[col];
        switch(c) {
            case '.':
                Lights_set(lights, row, col, OFF);
                break;
            case '#':
                Lights_set(lights, row, col, ON);
                break;
            case '\n':
                break;
            default:
                die("Unknown char '%c' in line '%s'", c, line);
                break;
        }
    }
}

static Lights *Lights_new_from_strings(size_t rows, size_t cols, char *lines[]) {
    Lights *self = Lights_new(rows, cols);

    for( int i = 0; lines[i] != NULL && i < self->max_rows; i++ ) {
        Lights_read_line(lines[i], self);
    }

    return self;
}

static Lights *Lights_new_from_fp(size_t rows, size_t cols, FILE *input) {
    Lights *self = Lights_new(rows, cols);

    foreach_line(input, Lights_read_line, self);

    return self;
}

static void Lights_destroy(Lights *self) {
    free(self->grid);
    free(self);
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

static inline int Lights_count(Lights *self, Light setting) {
    int num_lights = 0;
    
    for( int row = 0; row < self->rows; row++ ) {
        for( int col = 0; col < self->max_cols; col++ ) {
            if( Lights_get(self, row, col) == setting )
                num_lights++;
        }
    }

    return num_lights;
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

static int Lights_num_neighbors(Lights *self, size_t origin_row, size_t origin_col, Light setting) {
    int num_neighbors = 0;
    
    for( int i = -1; i <= 1; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            // We're not our own neighbor
            if( i == 0 && j == 0 )
                continue;
            
            int row = origin_row + i;
            int col = origin_col + j;

            // We've gone past the edge
            if( row < 0 || col < 0 ||
                row >= self->max_rows    ||
                col >= self->max_cols
            ) {
                continue;
            }

            if( Lights_get(self, row, col) == setting )
                num_neighbors++;
        }
    }

    return num_neighbors;
}

static void Lights_step(Lights *self) {
    Light *new_grid = Lights_new_grid(self);
    
    for( int row = 0; row < self->max_rows; row++ ) {
        for( int col = 0; col < self->max_cols; col++ ) {
            Light light = Lights_get(self, row, col);
            int num_on = Lights_num_neighbors(self, row, col, ON);

            // A light which is on stays on when 2 or 3 neighbors are on, and turns off otherwise.
            if( light ) {
                switch( num_on ) {
                    case 2:
                    case 3:
                        TWOD(new_grid, row, col, self->max_rows) = ON;
                        break;
                    default:
                        TWOD(new_grid, row, col, self->max_rows) = OFF;
                        break;
                }
            }
            // A light which is off turns on if exactly 3 neighbors are on, and stays off otherwise.
            else {
                if( num_on == 3 ) {
                    TWOD(new_grid, row, col, self->max_rows) = ON;
                }
                else {
                    TWOD(new_grid, row, col, self->max_rows) = OFF;
                }
            }
        }
    }

    free(self->grid);
    self->grid = new_grid;
}

static void test_lights_step() {
    printf("test_lights_step...");
    
    char *start[] = {
        ".#.#.#",
        "...##.",
        "#....#",
        "..#...",
        "#.#..#",
        "####.."
    };

    char *step1[] = {
        "..##..",
        "..##.#",
        "...##.",
        "......",
        "#.....",
        "#.##.."
    };

    char *step2[] = {
        "..###.",
        "......",
        "..###.",
        "......",
        ".#....",
        ".#...."
    };

    char *step3[] = {
        "...#..",
        "......",
        "...#..",
        "..##..",
        "......",
        "......"
    };

    char *step4[] = {
        "......",
        "......",
        "..##..",
        "..##..",
        "......",
        "......"
    };

    Lights *lights = Lights_new_from_strings(6, 6, start);

    g_assert_cmpint( Lights_num_neighbors(lights, 0, 0, ON),  ==, 1 );
    g_assert_cmpint( Lights_num_neighbors(lights, 0, 0, OFF), ==, 2 );
    g_assert_cmpint( Lights_num_neighbors(lights, 5, 5, ON),  ==, 1 );
    g_assert_cmpint( Lights_num_neighbors(lights, 5, 5, OFF), ==, 2 );
    g_assert_cmpint( Lights_num_neighbors(lights, 1, 1, ON),  ==, 2 );
    g_assert_cmpint( Lights_num_neighbors(lights, 1, 1, OFF), ==, 6 );

    Lights_step(lights);
    assert_lights_eq(lights, Lights_new_from_strings(6, 6, step1));

    Lights_step(lights);
    assert_lights_eq(lights, Lights_new_from_strings(6, 6, step2));

    Lights_step(lights);
    assert_lights_eq(lights, Lights_new_from_strings(6, 6, step3));

    Lights_step(lights);
    assert_lights_eq(lights, Lights_new_from_strings(6, 6, step4));

    Lights_destroy(lights);
    
    puts("OK");
}

static void test_read_lights() {
    printf("test_read_lights...");
    
    char *input[] = {
        ".#.#.#\n",
        "...##.\n",
        "#....#\n",
        "..#...\n",
        "#.#..#\n",
        "####..\n",
        NULL
    };

    Light want_array[6*6] = {
        OFF, ON, OFF, ON, OFF, ON,
        OFF, OFF, OFF, ON, ON, OFF,
        ON, OFF, OFF, OFF, OFF, ON,
        OFF, OFF, ON, OFF, OFF, OFF,
        ON, OFF, ON, OFF, OFF, ON,
        ON, ON, ON, ON, OFF, OFF
    };
    Lights *want = Lights_new_from_array_copy(6, 6, want_array);
    Lights *have = Lights_new_from_strings(6, 6, input);

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
    test_lights_step();
}

int main(int argc, char *argv[]) {
    if( argc == 1 ) {
        runtests();
    }
    else if( argc == 3 ) {
        int steps = atoi(argv[2]);
        FILE *input = open_file(argv[1], "r");
        Lights *lights = Lights_new_from_fp(100, 100, input);
        for( int i = 1; i <= steps; i++ ) {
            Lights_step(lights);
        }

        printf("%d\n", Lights_count(lights, ON));

        Lights_destroy(lights);
    }
    else {
        char *desc[] = {argv[0], "<input file>", "<num steps>"};
        usage(3, desc);
    }
    
    return 0;
}
