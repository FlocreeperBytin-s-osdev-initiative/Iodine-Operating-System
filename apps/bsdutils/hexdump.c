#include "bsdutils.h"
#include "../../fs/vfs.h"
#include "../../shell/commands.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/ctype.h"

void app_bsd_hexdump(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: hexdump [-C] file ...\n");
        return;
    }

    const char *file_path = argv[1];
    if (strcmp(argv[1], "-C") == 0) {
        if (argc < 3) {
            printf("usage: hexdump [-C] file ...\n");
            return;
        }
        file_path = argv[2];
    }

    vfs_node_t *node = vfs_resolve_path(file_path);
    if (!node || (node->flags & FS_DIRECTORY)) {
        printf("hexdump: %s: No such file\n", file_path);
        return;
    }

    uint8_t buf[16];
    uint32_t offset = 0;
    uint32_t bytes;

    while ((bytes = vfs_read(node, offset, 16, buf)) > 0) {
        printf("%08x  ", offset);

        for (uint32_t i = 0; i < 16; i++) {
            if (i < bytes) printf("%02x ", buf[i]);
            else printf("   ");
            if (i == 7) printf(" ");
        }

        printf(" |");
        for (uint32_t i = 0; i < bytes; i++) {
            putchar(isprint(buf[i]) ? buf[i] : '.');
        }
        printf("|\n");

        offset += bytes;
    }
    printf("%08x\n", offset);
}
