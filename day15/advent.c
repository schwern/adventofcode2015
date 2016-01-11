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

typedef struct {
    char *name;
    int capacity;
    int durability;
    int flavor;
    int texture;
    int calories;
} recipe_t;

static recipe_t* Recipe_new(char *name) {
    recipe_t *recipe = calloc(1, sizeof(recipe_t));
    recipe->name = strdup(name);

    return recipe;
}

static void Recipe_destroy(recipe_t *self) {
    free(self->name);
    free(self);
}

static void read_recipe( char *line, void *_recipes ) {
    GArray *recipes = (GArray *)_recipes;

    GMatchInfo *match;
    
    if( g_regex_match(Line_Re, line, 0, &match) ) {
        char *name  = g_match_info_fetch_named(match, "NAME");
        char *cap   = g_match_info_fetch_named(match, "CAP");
        char *dur   = g_match_info_fetch_named(match, "DUR");
        char *fla   = g_match_info_fetch_named(match, "FLA");
        char *tex   = g_match_info_fetch_named(match, "TEX");
        char *cal   = g_match_info_fetch_named(match, "CAL");

        recipe_t *recipe = Recipe_new(name);
        recipe->capacity        = atoi(cap);
        recipe->durability      = atoi(dur);
        recipe->flavor          = atoi(fla);
        recipe->texture         = atoi(tex);
        recipe->calories        = atoi(cal);

        g_array_append_val(recipes, recipe);

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

static inline GArray *Recipes_new() {
    return g_array_new(false, true, sizeof(recipe_t*));
}

static void Recipes_destroy(GArray *self, bool destroy_elements) {
    if( destroy_elements ) {
        for( int i = 0; i < self->len; i++ ) {
            Recipe_destroy( g_array_index(self, recipe_t*, i) );
        }
    }
    g_array_free(self, true);
}

static void test_recipe_score() {
    printf("test_recipe_score...");
    
    int num_lines = 2;
    char *lines[] = {
        "Butterscotch: capacity -1, durability -2, flavor 6, texture 3, calories 8\n",
        "Cinnamon: capacity 2, durability 3, flavor -2, texture -1, calories 3\n"
    };

    GArray *recipes = Recipes_new();
    for( int i = 0; i < num_lines; i++ ) {
        read_recipe(lines[i], recipes);
    }

    recipe_t *butterscotch = g_array_index(recipes, recipe_t*, 0);
    assert( streq(butterscotch->name, "Butterscotch") );
    assert( butterscotch->capacity        == -1 );
    assert( butterscotch->durability      == -2 );
    assert( butterscotch->flavor          == 6 );
    assert( butterscotch->texture         == 3 );
    assert( butterscotch->calories        == 8 );

    recipe_t *cinnamon = g_array_index(recipes, recipe_t*, 1);
    assert( streq(cinnamon->name, "Cinnamon") );
    assert( cinnamon->capacity        == 2 );
    assert( cinnamon->durability      == 3 );
    assert( cinnamon->flavor          == -2 );
    assert( cinnamon->texture         == -1 );
    assert( cinnamon->calories        == 3 );

    Recipes_destroy(recipes, true);
    
    puts("OK");
}

static void runtests() {
    test_recipe_score();
}

int main(int argc, char *argv[]) {
    init_regexes();

    runtests();
}
