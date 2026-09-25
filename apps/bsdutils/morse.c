#include "bsdutils.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/ctype.h"

static const char *morse_table[26] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
    "..-", "...-", ".--", "-..-", "-.--", "--.."
};

void app_morse(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: morse [string ...]\n");
        return;
    }

    for (int a = 1; a < argc; a++) {
        const char *w = argv[a];
        for (int i = 0; w[i]; i++) {
            char ch = tolower(w[i]);
            if (ch >= 'a' && ch <= 'z') {
                printf("%s ", morse_table[ch - 'a']);
            } else if (ch >= '0' && ch <= '9') {
                printf("...-- ");
            } else if (isspace(ch)) {
                printf("  ");
            }
        }
        printf("  ");
    }
    printf("\n");
}
