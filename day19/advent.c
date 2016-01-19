#include "common.h"
#include <glib.h>
#include "fmemopen.h"

typedef GHashTable Transforms;

static void Transforms_free_element(void *_el) {
    char *el = *(char **)_el;

    free(el);
}

static void Transforms_free_value(void *_val) {
    GArray *val = (GArray *)_val;

    g_array_free(val, true);
}

static inline Transforms *Transforms_new() {
    return g_hash_table_new_full(g_str_hash, g_str_equal, free, Transforms_free_value);
}

static inline void Transforms_destroy(Transforms *self) {
    g_hash_table_unref(self);
}

static inline GArray *Transforms_lookup(Transforms *self, char *in) {
    return g_hash_table_lookup(self, in);
}

static GArray *Transforms_lookup_or_add(Transforms *self, char *in) {
    GArray *outs = g_hash_table_lookup(self, in);

    if( outs == NULL ) {
        outs = g_array_new(false, false, sizeof(char*));
        g_array_set_clear_func(outs, Transforms_free_element);
        
        g_hash_table_insert(self, strdup(in), outs);
    }

    return outs;
}    

static void Transforms_add(Transforms *self, char *in, char *out) {
    GArray *outs = Transforms_lookup_or_add(self, in);
    g_array_append_val(outs, out);
}

static void read_transform_line(char *line, void *_trans) {
    Transforms *trans = (Transforms *)_trans;

    char *in  = calloc(4, sizeof(char));
    char *out = calloc(20, sizeof(char));
    sscanf(line, "%3s => %20s\n", in, out);

    Transforms_add(trans, in, out);

    free(in);
}

static Transforms *read_transforms(FILE *input) {
    Transforms *trans = Transforms_new();
    foreach_line(input, read_transform_line, trans);

    return trans;
}

static void test_read_transforms() {
    printf("test_read_transforms...");

    char *input = {
        "H => HO\n"
        "H => OH\n"
        "O => HH\n"
    };

    FILE *fp = fmemopen(input, strlen(input), "r");
    Transforms *trans = read_transforms(fp);

    g_assert_true( Transforms_lookup(trans, "H") );
    g_assert_true( Transforms_lookup(trans, "O") );
    g_assert_false( Transforms_lookup(trans, "E") );

    GArray *H = Transforms_lookup(trans, "H");
    GArray *O = Transforms_lookup(trans, "O");
    g_assert_cmpint( H->len, ==, 2 );
    g_assert_cmpstr( g_array_index( H, char *, 0 ), ==, "HO" );
    g_assert_cmpstr( g_array_index( H, char *, 1 ), ==, "OH" );
    g_assert_cmpint( O->len, ==, 1 );
    g_assert_cmpstr( g_array_index( O, char *, 0 ), ==, "HH" );
    
    Transforms_destroy(trans);
    fclose(fp);
    
    puts("OK");
}

static GHashTable *make_all_molecules(Transforms *trans, char *start) {
    GHashTable *molecules = g_hash_table_new_full( g_str_hash, g_str_equal, free, NULL );
    GList *keys = g_hash_table_get_keys(trans);
    for( GList *key = keys; key != NULL; key = key->next ) {
        char *k = key->data;
        GArray *vals = g_hash_table_lookup(trans, k);

        for( int i = 0; i < vals->len; i++ ) {
            char *v = g_array_index(vals, char *, i);
            
            char *pos = start;
            while( (pos = strstr(pos, k)) != NULL ) {
                size_t newlen = strlen(start) + strlen(v) - strlen(k) + 1;

                char *new = calloc( newlen, sizeof(char) );
                strncat(new, start, pos - start);
                strcat(new, v);
                strcat(new, pos + strlen(k));

                g_hash_table_add(molecules, new);

                pos += strlen(k);
            }
        }
    }

    g_list_free(keys);

    return molecules;
}

static int num_replacements(Transforms *trans, char *start) {
    GHashTable *molecules = make_all_molecules(trans, start);
    int num = g_hash_table_size(molecules);

    g_hash_table_unref(molecules);
    
    return num;
}


static void test_num_replacements() {
    printf("test_num_replacements...");

    char *input = {
        "H => HO\n"
        "H => OH\n"
        "O => HH\n"
    };

    FILE *fp = fmemopen(input, strlen(input), "r");
    Transforms *trans = read_transforms(fp);

    g_assert_cmpint( num_replacements(trans, "HOH"), ==, 4 );

    Transforms_destroy(trans);
    fclose(fp);
    
    puts("OK");
}

static void runtests() {
    test_read_transforms();
    test_num_replacements();
}

int main(int argc, char *argv[]) {
    if( argc == 3 ) {
        FILE *input = open_file(argv[1], "r");
        Transforms *trans = read_transforms(input);
        printf("%d\n", num_replacements(trans, argv[2]));
        Transforms_destroy(trans);
    }
    else if( argc == 1 ) {
        runtests();
    }
    else {
        char *desc[] = {argv[0], "<input file>", "<start>"};
        usage(3, desc);
    }

    return 0;
}
