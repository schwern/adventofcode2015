#include "file.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mine_adventcoins(char *start) {
    char key[20] = "";

    for(int i = 0; i < INT_MAX; i++ ) {
        char num[10];
        sprintf(num, "%d", i);
        
        strlcpy(key, start, 20);
        strlcat(key, num, 20);

        char *md5sum = g_compute_checksum_for_string(G_CHECKSUM_MD5, key, -1);
        if( strncmp(md5sum, "00000", 5) == 0 )
            return i;
    }

    return -1;
}

int main(const int argc, char **argv) {
    char *argv_desc[2] = { argv[0], "<secret key>" };
    if( !usage(argc, 2, argv_desc) ) {
        return -1;
    }

    int coin = mine_adventcoins(argv[1]);
    printf("%d\n", coin);
    
    return 0;
}
