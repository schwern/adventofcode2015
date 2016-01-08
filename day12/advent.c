#include "common.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <json-glib/json-glib.h>
#include <gio/gunixinputstream.h>

static int sum_json_node(JsonNode *node);

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

static void json_object_foreach_should_ignore(
    JsonObject *object,
    const char *name,
    JsonNode *node,
    gpointer _should_ignore
) {
    bool *should_ignore = (bool *)_should_ignore;

    if( *should_ignore )
        return;

    if( JSON_NODE_HOLDS_VALUE(node) ) {
        const char *value = json_node_get_string(node);
        if( value && streq( value, "red" ) )
            *should_ignore = true;
    }
}

static bool should_ignore_object(JsonObject *object) {
    bool should_ignore = false;

    json_object_foreach_member( object, json_object_foreach_should_ignore, &should_ignore );

    return should_ignore;
}

static void json_object_foreach_sum(
    JsonObject *object,
    const char *name,
    JsonNode *node,
    gpointer _sum
) {
    int *sum = (int *)_sum;

    *sum += sum_json_node(node);
}


static int sum_json_object(JsonObject *object) {
    int sum = 0;
    
    if( should_ignore_object( object ) )
        return 0;

    json_object_foreach_member( object, json_object_foreach_sum, &sum );

    return sum;
}

static void json_array_foreach_sum(JsonArray *array, guint index, JsonNode *node, gpointer _sum) {
    int *sum = (int *)_sum;

    *sum += sum_json_node(node);
}
    
static int sum_json_array(JsonArray *array) {
    int sum = 0;
    
    json_array_foreach_element(array, json_array_foreach_sum, &sum);

    return sum;
}

static int sum_json_node(JsonNode *node) {
    switch(JSON_NODE_TYPE(node)) {
        case JSON_NODE_OBJECT:
            return sum_json_object( json_node_get_object(node) );
        case JSON_NODE_ARRAY:
            return sum_json_array( json_node_get_array(node) );
        case JSON_NODE_VALUE:
            return json_node_get_int(node);
        default:
            return 0;
    }
}

static int sum_json_string( char *input ) {
    GError *error = NULL;
    JsonParser *parser = json_parser_new();
    
    if( !json_parser_load_from_data(parser, input, -1, &error) )
        die("Could not load JSON: %s", error->message);

    JsonNode *root = json_parser_get_root(parser); 

    int sum = sum_json_node(root);
    
    g_object_unref(parser);

    return sum;
}

static void test_sum_json_string() {
    char *tests[] = {
        "[1,2,3]",                                      "6",
        "[1,\"red\",5]",                                "6",
        "[1, [2, 3], \"four\", 5]",                     "11",
        "[1,{\"c\":\"red\",\"b\":2},3]",                "4",
        "{\"d\":\"red\",\"e\":[1,2,3,4],\"f\":5}",      "0",
        ""
    };

    printf("Testing sum_json_string()\n");
    for( int i = 0; !is_empty( tests[i] ); i+=2 ) {
        char *arg = tests[i];
        int want  = atoi(tests[i+1]);
        int have  = sum_json_string(arg);
        
        printf("\tsum_json_string(%s), have = %d, want = %d\n", arg, have, want);

        assert( have == want );
    }
}

static void tests() {
    test_sum_all_numbers();
    test_sum_json_string();
}

static int sum_json(FILE *input) {
    GError *error = NULL;
    JsonParser *parser = json_parser_new();
    GInputStream *stream = g_unix_input_stream_new( fileno(input), false );
    
    if( !json_parser_load_from_stream(parser, stream, NULL, &error) )
        die("Could not load JSON: %s", error->message);

    JsonNode *root = json_parser_get_root(parser);

    int sum = sum_json_node(root);
    
    g_object_unref(parser);
    g_object_unref(stream);

    return sum;
}

int main(int argc, char **argv) {
    if( argc == 1 ) {
        tests();
    }
    else if( argc == 2 ) {
        FILE *input = open_file(argv[1], "r");
        int sum = sum_json(input);
        printf("%d\n", sum);
    }
    else {
        char *desc[] = {argv[0], "<input file>"};
        usage(2, desc);
    }
}
