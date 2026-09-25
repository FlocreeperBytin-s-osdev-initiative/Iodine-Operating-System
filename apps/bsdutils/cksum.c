#include "bsdutils.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/ctype.h"
#include "../../fs/vfs.h"

void app_cksum(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: cksum [file ...]\n");
        return;
    }

    // POSIX 1003.2 32-bit CRC checksum
    uint32_t crc_table[256];
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i << 24;
        for (int j = 0; j < 8; j++) {
            c = (c & 0x80000000) ? ((c << 1) ^ 0x04C11DB7) : (c << 1);
        }
        crc_table[i] = c;
    }

    for (int a = 1; a < argc; a++) {
        extern const char *shell_get_cwd(void);
        char full_path[VFS_PATH_MAX];
        if (argv[a][0] == '/') {
            strncpy(full_path, argv[a], VFS_PATH_MAX - 1);
        } else {
            const char *cwd = shell_get_cwd();
            if (strcmp(cwd, "/") == 0) {
                snprintf(full_path, VFS_PATH_MAX, "/%s", argv[a]);
            } else {
                snprintf(full_path, VFS_PATH_MAX, "%s/%s", cwd, argv[a]);
            }
        }

        vfs_node_t *node = vfs_resolve_path(full_path);
        if (!node) {
            printf("cksum: %s: No such file\n", argv[a]);
            continue;
        }

        uint32_t crc = 0;
        uint32_t total = 0;
        uint8_t buf[256];
        uint32_t bytes;

        while ((bytes = vfs_read(node, total, sizeof(buf), buf)) > 0) {
            for (uint32_t i = 0; i < bytes; i++) {
                crc = (crc << 8) ^ crc_table[((crc >> 24) ^ buf[i]) & 0xFF];
            }
            total += bytes;
        }

        uint32_t len_val = total;
        while (len_val > 0) {
            crc = (crc << 8) ^ crc_table[((crc >> 24) ^ (len_val & 0xFF)) & 0xFF];
            len_val >>= 8;
        }
        crc = ~crc;

        printf("%u %u %s\n", crc, total, argv[a]);
    }
}

void app_whoami(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("root\n");
}
