#include "common.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    int string_size;
    int mem_size;
    int encoding_size;
} StringInfo;

static void string_info(char *line, void *_info) {
    StringInfo *info = (StringInfo *)_info;
    StringInfo lineinfo = { .string_size = 0, .mem_size = 0, .encoding_size = 2 };

    for( const char *pos = line; *pos; pos++ ) {
        switch(pos[0]) {
            case '\\':
                lineinfo.string_size++;
                lineinfo.encoding_size+=2;
                pos++;
                switch(pos[0]) {
                    case 'x':
                        lineinfo.string_size += 3;
                        lineinfo.mem_size++;
                        lineinfo.encoding_size += 3;
                        pos += 2;
                        break;
                    default:
                        lineinfo.string_size++;
                        lineinfo.mem_size++;
                        lineinfo.encoding_size += 2;
                        break;
                }
                break;
            case '"':
                lineinfo.string_size++;
                lineinfo.encoding_size+=2;
                break;
            case '\n':
            case ' ':
                break;
            default:
                lineinfo.string_size++;
                lineinfo.mem_size++;
                lineinfo.encoding_size++;
                break;
        }
    }

    info->string_size   += lineinfo.string_size;
    info->mem_size      += lineinfo.mem_size;
    info->encoding_size += lineinfo.encoding_size;
}

static StringInfo read_strings(FILE *input) {
    StringInfo info = { .string_size = 0, .mem_size = 0 };
    
    foreach_line(input, string_info, &info);
    
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
