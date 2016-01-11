#include "common.h"
#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <stdbool.h>

GRegex *Line_Re;

static void free_regexes() {
    g_regex_unref(Line_Re);
}

static void init_regexes() {
    if( !Line_Re ) {
        Line_Re = compile_regex(
            "^"
            "(?<NAME>[[:alpha:]]+): capacity (?<CAP>-?\\d+), durability (?<DUR>-?\\d+), flavor (?<FLA>-?\\d+), texture (?<TEX>-?\\d+), calories (?<CAL>-?\\d+)"
            "$",
            G_REGEX_OPTIMIZE,
            0
        );
        atexit(free_regexes);
    }
}

typedef enum {
    CAPACITY, DURABILITY, FLAVOR, TEXTURE, CALORIES, NUM_PROP_TYPES
} prop_types;

typedef struct {
    char *name;
    int props[NUM_PROP_TYPES];
} ingredient_t;

static ingredient_t* Ingredient_new(char *name) {
    ingredient_t *ingredient = calloc(1, sizeof(ingredient_t));
    ingredient->name = strdup(name);

    return ingredient;
}

static void Ingredient_destroy(ingredient_t *self) {
    free(self->name);
    free(self);
}

static void read_ingredient( char *line, void *_ingredients ) {
    GArray *ingredients = (GArray *)_ingredients;

    GMatchInfo *match;
    
    if( g_regex_match(Line_Re, line, 0, &match) ) {
        char *name  = g_match_info_fetch_named(match, "NAME");
        char *cap   = g_match_info_fetch_named(match, "CAP");
        char *dur   = g_match_info_fetch_named(match, "DUR");
        char *fla   = g_match_info_fetch_named(match, "FLA");
        char *tex   = g_match_info_fetch_named(match, "TEX");
        char *cal   = g_match_info_fetch_named(match, "CAL");

        ingredient_t *ingredient = Ingredient_new(name);
        ingredient->props[CAPACITY]     = atoi(cap);
        ingredient->props[DURABILITY]   = atoi(dur);
        ingredient->props[FLAVOR]       = atoi(fla);
        ingredient->props[TEXTURE]      = atoi(tex);
        ingredient->props[CALORIES]     = atoi(cal);

        g_array_append_val(ingredients, ingredient);

        g_match_info_free(match);
        free(name);
        free(cap);
        free(dur);
        free(fla);
        free(tex);
        free(cal);
    }
    else if( !is_blank(line) ) {
        die("Unknown line: '%s'");
    }
}

static inline GArray *Ingredients_new() {
    return g_array_new(false, true, sizeof(ingredient_t*));
}

static void Ingredients_destroy(GArray *self, bool destroy_elements) {
    if( destroy_elements ) {
        for( int i = 0; i < self->len; i++ ) {
            Ingredient_destroy( g_array_index(self, ingredient_t*, i) );
        }
    }
    g_array_free(self, true);
}

static inline ingredient_t *Ingredients_get(GArray *self, size_t index) {
    return g_array_index(self, ingredient_t*, index);
}

static inline long property_score(GArray *ingredients, int measures[], int property) {
    long score = 0;
    for( int i = 0; i < ingredients->len; i++ ) {
        score += measures[i] * Ingredients_get(ingredients, i)->props[property];
    }

    return score < 0 ? 0 : score;
}

static long recipe_score(GArray *ingredients, int *measures) {
    long score = 1;

    score *= property_score(ingredients, measures, CAPACITY);
    score *= property_score(ingredients, measures, DURABILITY);
    score *= property_score(ingredients, measures, FLAVOR);
    score *= property_score(ingredients, measures, TEXTURE);

    return score;
}

static bool increment_measures( int *measures, int num, int max ) {
    int sum = 0;
    int end = num-1;

    for( int i = end; i >= 0; i-- ) {
        sum += measures[i];
        if( sum == max ) {
            /* Done */
            if( i == end )
                return false;

            /* Hand a digit up */
            measures[i]--;
            measures[i+1]++;

            if( measures[i+1] == max )
                return true;

            /* Carry operation */
            if( i != 0 ) {
                measures[0] = measures[i];
                measures[i] = 0;
            }

            return true;
        }
    }

    return true;
}

static long best_ingredients_combo( GArray *ingredients, int total ) {
    size_t num = ingredients->len;
    int *measures = calloc(num, sizeof(int));
    long score = 0;

    measures[0] = total;
    do {
        score = MAX(score, recipe_score(ingredients, measures));
    } while( increment_measures(measures, num, total) );

    free(measures);

    return score;
}

static GArray *read_ingredients(FILE *input) {
    GArray *ingredients = Ingredients_new();
    foreach_line(input, read_ingredient, ingredients);
    return ingredients;
}

static void test_ingredients() {
    printf("test_ingredient_score...");
    
    int num_lines = 2;
    char *lines[] = {
        "Butterscotch: capacity -1, durability -2, flavor 6, texture 3, calories 8\n",
        "Cinnamon: capacity 2, durability 3, flavor -2, texture -1, calories 3\n"
    };

    GArray *ingredients = Ingredients_new();
    for( int i = 0; i < num_lines; i++ ) {
        read_ingredient(lines[i], ingredients);
    }

    ingredient_t *butterscotch = g_array_index(ingredients, ingredient_t*, 0);
    assert( streq(butterscotch->name, "Butterscotch") );
    assert( butterscotch->props[CAPACITY]        == -1 );
    assert( butterscotch->props[DURABILITY]      == -2 );
    assert( butterscotch->props[FLAVOR]          == 6 );
    assert( butterscotch->props[TEXTURE]         == 3 );
    assert( butterscotch->props[CALORIES]        == 8 );

    ingredient_t *cinnamon = g_array_index(ingredients, ingredient_t*, 1);
    assert( streq(cinnamon->name, "Cinnamon") );
    assert( cinnamon->props[CAPACITY]        == 2 );
    assert( cinnamon->props[DURABILITY]      == 3 );
    assert( cinnamon->props[FLAVOR]          == -2 );
    assert( cinnamon->props[TEXTURE]         == -1 );
    assert( cinnamon->props[CALORIES]        == 3 );

    int measures[] = {44, 56};    
    assert( property_score(ingredients, measures, CAPACITY) == 68 );
    assert( recipe_score(ingredients, measures) == 62842880 );

    /* Negative scores are 0 */
    measures[0] = 100;
    measures[1] = 0;
    assert( property_score(ingredients, measures, CAPACITY) == 0 );
    assert( recipe_score(ingredients, measures) == 0 );
    
    assert( best_ingredients_combo(ingredients, 100) == 62842880 );
    
    Ingredients_destroy(ingredients, true);
    
    puts("OK");
}

static void test_increment_measures() {
    printf("test_increment_measures...");

    /* Starting state */
    int have[4] = {5, 0, 0};
    assert( increment_measures( have, 3, 5 ) );
    assert( have[0] == 4 );
    assert( have[1] == 1 );
    assert( have[2] == 0 );

    /* Carry */
    have[0] = 0;
    have[1] = 4;
    have[2] = 1;
    assert( increment_measures( have, 3, 5 ) );
    assert( have[0] == 3 );
    assert( have[1] == 0 );
    assert( have[2] == 2 );
    
    /* 2nd to last state */
    have[0] = 0;
    have[1] = 1;
    have[2] = 4;
    assert( increment_measures( have, 3, 5 ) );
    assert( have[0] == 0 );
    assert( have[1] == 0 );
    assert( have[2] == 5 );
    
    /* End state */
    have[0] = 0;
    have[1] = 0;
    have[2] = 5;
    assert( !increment_measures( have, 3, 5 ) );
    assert( have[0] == 0 );
    assert( have[1] == 0 );
    assert( have[2] == 5 );

    puts("OK");

    have[0] = 0;
    have[1] = 0;
    have[2] = 5;
    have[3] = 0;
    assert( increment_measures( have, 4, 5 ) );
    assert( have[0] == 4 );
    assert( have[1] == 0 );
    assert( have[2] == 0 );
    assert( have[3] == 1 );
}

static void runtests() {
    test_ingredients();
    test_increment_measures();
}

int main(int argc, char *argv[]) {
    init_regexes();

    if( argc == 1 ) {
        runtests();
    }
    else if( argc == 3 ) {
        FILE *input = open_file(argv[1], "r");
        GArray *ingredients = read_ingredients(input);
        printf("%ld\n", best_ingredients_combo(ingredients, atoi(argv[2])));
        Ingredients_destroy(ingredients, true);
    }
    else {
        char *desc[] = {argv[0], "<input file>", "<total units>"};
        usage(3, desc);
    }
}
