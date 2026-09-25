#include "bsdutils.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"

void app_column(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: column [-t] [-s delim] [words...]\n");
        return;
    }

    int col_width = 16;
    int current_col = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) continue;
        printf("%-16s", argv[i]);
        current_col += col_width;
        if (current_col >= 64) {
            printf("\n");
            current_col = 0;
        }
    }
    if (current_col != 0) {
        printf("\n");
    }
}
