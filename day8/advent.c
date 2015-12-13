#include "common.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    int string_size;
    int mem_size;
    int encoding_size;
} StringInfo;

static StringInfo string_info(char *line) {
    StringInfo info = { .string_size = 0, .mem_size = 0, .encoding_size = 2 };
    char *pos;

    for( pos = line; *pos; *pos++ ) {
        switch(pos[0]) {
            case '\\':
                info.string_size++;
                info.encoding_size+=2;
                pos++;
                switch(pos[0]) {
                    case 'x':
                        info.string_size += 3;
                        info.mem_size++;
                        info.encoding_size += 3;
                        pos += 2;
                        break;
                    default:
                        info.string_size++;
                        info.mem_size++;
                        info.encoding_size += 2;
                        break;
                }
                break;
            case '"':
                info.string_size++;
                info.encoding_size+=2;
                break;
            case '\n':
            case ' ':
                break;
            default:
                info.string_size++;
                info.mem_size++;
                info.encoding_size++;
                break;
        }
    }

    return info;
}

static StringInfo read_strings(FILE *input) {
    char *line = NULL;
    size_t linelen = 0;
    StringInfo info = { .string_size = 0, .mem_size = 0 };
    
    while( getline(&line, &linelen, input) > 0 ) {
        StringInfo lineinfo = string_info(line);
        info.string_size += lineinfo.string_size;
        info.mem_size    += lineinfo.mem_size;
        info.encoding_size += lineinfo.encoding_size;
    }
    free(line);
    
    return info;
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    
    if( argc > 2 ) {
        char *desc[2] = {argv[0], "<inputfile>"};
        usage(2, desc);
    }

    if( argc >= 2 ) {
        input = open_file(argv[1], "r");
    }

    StringInfo info = read_strings(input);
    printf("%d - %d = %d\n", info.string_size, info.mem_size, info.string_size - info.mem_size);
    printf("%d - %d = %d\n", info.encoding_size, info.string_size, info.encoding_size - info.string_size);
    
    return 0;
}
