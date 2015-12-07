#include "file.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

static void free_key(gpointer data) {
    free(data);
}

static gint64 *house_key(const int *pos) {
    gint64 *key = malloc(sizeof(gint64));

    *key = pos[0];
    *key += (gint64)pos[1] * 0xffffffffL;

    return key;
}

static void deliver( GHashTable *houses, const int *pos ) {
    gint64 *key = house_key(pos);

    g_hash_table_add(houses, key);
}

static GHashTable *deliver_to_houses( FILE *fp ) {
    GHashTable *houses = g_hash_table_new_full(g_int64_hash, g_int64_equal, free_key, NULL);
    int pos[2][2] = {{0,0}, {0,0}};
    int steps = 0;
    
    deliver(houses, pos[0]);
    
    while( !feof(fp) ) {
        short who = steps % 2;
        
        char c = (char)fgetc(fp);
        switch(c) {
            case '>':
                pos[who][0]++;
                deliver(houses, pos[who]);
                break;
            case '<':
                pos[who][0]--;
                deliver(houses, pos[who]);
                break;
            case 'v':
                pos[who][1]--;
                deliver(houses, pos[who]);
                break;
            case '^':
                pos[who][1]++;
                deliver(houses, pos[who]);
                break;
        }

        steps++;
    };

    return houses;
}

int main(const int argc, char **argv) {
    char *argv_desc[2] = { argv[0], "<inputfile>" };
    if( !usage(argc, 2, argv_desc) ) {
        return -1;
    }

    FILE *fp = open_file(argv[1], "r");

    GHashTable *houses = deliver_to_houses(fp);
    printf("%u\n", g_hash_table_size(houses));

    g_hash_table_destroy(houses);
    
    return 0;
}
