#include "bsdutils.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/ctype.h"

void app_banner(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: banner string ...\n");
        return;
    }

    for (int a = 1; a < argc; a++) {
        const char *word = argv[a];
        printf("\n");
        for (int row = 0; row < 5; row++) {
            printf("  ");
            for (int i = 0; word[i]; i++) {
                char ch = toupper(word[i]);
                if (ch == 'I') {
                    if (row == 0 || row == 4) printf("##### ");
                    else                      printf("  #   ");
                } else if (ch == 'O') {
                    if (row == 0 || row == 4) printf(" ###  ");
                    else                      printf("#   # ");
                } else if (ch == 'D') {
                    if (row == 0 || row == 4) printf("####  ");
                    else                      printf("#   # ");
                } else if (ch == 'N') {
                    if (row == 0)             printf("#   # ");
                    else if (row == 1)        printf("##  # ");
                    else if (row == 2)        printf("# # # ");
                    else if (row == 3)        printf("#  ## ");
                    else                      printf("#   # ");
                } else if (ch == 'E') {
                    if (row == 0 || row == 4) printf("##### ");
                    else if (row == 2)        printf("####  ");
                    else                      printf("#     ");
                } else if (ch == 'S') {
                    if (row == 0 || row == 2 || row == 4) printf(" #### ");
                    else if (row == 1)                    printf("#     ");
                    else                                  printf("    # ");
                } else {
                    printf("%c ", ch);
                }
            }
            printf("\n");
        }
        printf("\n");
    }
}
