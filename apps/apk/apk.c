#include "apk.h"
#include "../../fs/vfs.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/stdlib.h"

typedef struct {
    const char *name;
    const char *version;
    const char *description;
    const char *license;
    uint32_t size_kb;
    const char *files[4];
    bool installed;
} apk_package_t;

static apk_package_t repo_packages[] = {
    {
        "musl", "1.2.4-r0",
        "Lightweight, fast, standard-compliant POSIX C library",
        "MIT", 820,
        { "/lib/ld-musl-x86_64.so.1", "/lib/libc.musl.so", NULL, NULL },
        true
    },
    {
        "bsdutils", "3.2.0-r1",
        "Classic BSD utilities (hexdump, cal, column, banner, cksum)",
        "BSD-2-Clause", 412,
        { "/bin/hexdump", "/bin/cal", "/bin/column", "/bin/banner" },
        true
    },
    {
        "virtio-tools", "1.0.0-r0",
        "VirtIO hardware device probe and control utilities",
        "BSD-2-Clause", 215,
        { "/bin/virtio-blk", "/bin/virtio-net", "/bin/virtio-pci", NULL },
        true
    },
    {
        "ldk-sandbox", "1.0.0-r0",
        "Linux Driver Kit isolated microkernel driver sandbox",
        "BSD-2-Clause / GPLv2-Isolated", 1024,
        { "/bin/ldk-sandbox", "/etc/ldk.conf", "/dev/ldk0", NULL },
        true
    },
    {
        "curl", "8.2.1-r0",
        "Command line tool for transferring data with URL syntax",
        "curl", 512,
        { "/bin/curl", NULL, NULL, NULL },
        false
    },
    {
        "lua", "5.4.6-r0",
        "Lightweight embeddable scripting language runtime",
        "MIT", 340,
        { "/bin/lua", "/bin/luac", NULL, NULL },
        false
    },
    {
        "busybox-posix", "1.36.1-r0",
        "POSIX compliant core utilities toolkit",
        "GPLv2-Isolated", 960,
        { "/bin/sh", "/bin/grep", "/bin/sed", "/bin/awk" },
        false
    }
};

#define REPO_PKG_COUNT (sizeof(repo_packages) / sizeof(repo_packages[0]))

static void apk_update(void) {
    printf("fetch http://packages.iodine-os.org/v1.0/main/x86_64/APKINDEX.tar.gz\n");
    printf("fetch http://packages.iodine-os.org/v1.0/community/x86_64/APKINDEX.tar.gz\n");
    printf("v1.0.0-18-g2a3f [http://packages.iodine-os.org/v1.0/main]\n");
    printf("OK: %u distinct packages available\n", (unsigned int)REPO_PKG_COUNT);
}

static void apk_list(void) {
    for (size_t i = 0; i < REPO_PKG_COUNT; i++) {
        printf("%-16s %-10s %s %s\n",
               repo_packages[i].name,
               repo_packages[i].version,
               repo_packages[i].installed ? "[installed]" : "           ",
               repo_packages[i].description);
    }
}

static void apk_info(const char *pkg_name) {
    if (!pkg_name) {
        // List installed packages
        for (size_t i = 0; i < REPO_PKG_COUNT; i++) {
            if (repo_packages[i].installed) {
                printf("%s-%s description:\n%s\n\n",
                       repo_packages[i].name, repo_packages[i].version, repo_packages[i].description);
            }
        }
        return;
    }

    for (size_t i = 0; i < REPO_PKG_COUNT; i++) {
        if (strcmp(repo_packages[i].name, pkg_name) == 0) {
            printf("%s-%s description:\n%s\n\n",
                   repo_packages[i].name, repo_packages[i].version, repo_packages[i].description);
            printf("webpage: https://iodine-os.org/packages/%s\n", repo_packages[i].name);
            printf("installed size: %u KiB\n", repo_packages[i].size_kb);
            printf("license: %s\n", repo_packages[i].license);
            printf("status: %s\n", repo_packages[i].installed ? "installed" : "available");
            printf("files:\n");
            for (int f = 0; f < 4 && repo_packages[i].files[f]; f++) {
                printf("  %s\n", repo_packages[i].files[f]);
            }
            return;
        }
    }
    printf("ERROR: package '%s' not found\n", pkg_name);
}

static void apk_add(const char *pkg_name) {
    if (!pkg_name) {
        printf("Usage: apk add <package>\n");
        return;
    }

    for (size_t i = 0; i < REPO_PKG_COUNT; i++) {
        if (strcmp(repo_packages[i].name, pkg_name) == 0) {
            if (repo_packages[i].installed) {
                printf("OK: %s-%s already installed.\n", repo_packages[i].name, repo_packages[i].version);
                return;
            }

            printf("(1/1) Installing %s (%s)\n", repo_packages[i].name, repo_packages[i].version);
            for (int f = 0; f < 4 && repo_packages[i].files[f]; f++) {
                vfs_create_file(repo_packages[i].files[f]);
            }
            repo_packages[i].installed = true;

            uint32_t total_kb = 0;
            int count = 0;
            for (size_t j = 0; j < REPO_PKG_COUNT; j++) {
                if (repo_packages[j].installed) {
                    total_kb += repo_packages[j].size_kb;
                    count++;
                }
            }
            printf("Executing busybox-1.36.1-r0.trigger\n");
            printf("OK: %u KiB in %d packages\n", total_kb, count);
            return;
        }
    }
    printf("ERROR: unable to select packages:\n  %s (no such package)\n", pkg_name);
}

static void apk_del(const char *pkg_name) {
    if (!pkg_name) {
        printf("Usage: apk del <package>\n");
        return;
    }

    for (size_t i = 0; i < REPO_PKG_COUNT; i++) {
        if (strcmp(repo_packages[i].name, pkg_name) == 0) {
            if (!repo_packages[i].installed) {
                printf("WARNING: %s is not installed\n", pkg_name);
                return;
            }

            printf("(1/1) Purging %s (%s)\n", repo_packages[i].name, repo_packages[i].version);
            for (int f = 0; f < 4 && repo_packages[i].files[f]; f++) {
                vfs_unlink(repo_packages[i].files[f]);
            }
            repo_packages[i].installed = false;

            uint32_t total_kb = 0;
            int count = 0;
            for (size_t j = 0; j < REPO_PKG_COUNT; j++) {
                if (repo_packages[j].installed) {
                    total_kb += repo_packages[j].size_kb;
                    count++;
                }
            }
            printf("OK: %u KiB in %d packages\n", total_kb, count);
            return;
        }
    }
    printf("ERROR: package '%s' not found\n", pkg_name);
}

static void apk_search(const char *query) {
    int found = 0;
    for (size_t i = 0; i < REPO_PKG_COUNT; i++) {
        if (!query || strstr(repo_packages[i].name, query) || strstr(repo_packages[i].description, query)) {
            printf("%s-%s - %s\n", repo_packages[i].name, repo_packages[i].version, repo_packages[i].description);
            found++;
        }
    }
    if (found == 0) {
        printf("No packages found matching '%s'\n", query);
    }
}

void app_apk(int argc, char **argv) {
    if (argc < 2) {
        printf("apk-tools 2.14.0, compiled for Iodine OS (x86_64/i386)\n\n");
        printf("Usage: apk [<options>] <command> [<arguments> ...]\n\n");
        printf("Commands:\n");
        printf("  add        Add packages to World and commit changes\n");
        printf("  del        Remove packages from World and commit changes\n");
        printf("  update     Update repository package indexes\n");
        printf("  upgrade    Upgrade installed packages\n");
        printf("  list       List packages matching pattern\n");
        printf("  info       Give detailed information about package\n");
        printf("  search     Search package by name or description\n");
        return;
    }

    const char *cmd = argv[1];
    if (strcmp(cmd, "add") == 0) {
        apk_add(argc > 2 ? argv[2] : NULL);
    } else if (strcmp(cmd, "del") == 0) {
        apk_del(argc > 2 ? argv[2] : NULL);
    } else if (strcmp(cmd, "update") == 0) {
        apk_update();
    } else if (strcmp(cmd, "list") == 0) {
        apk_list();
    } else if (strcmp(cmd, "info") == 0) {
        apk_info(argc > 2 ? argv[2] : NULL);
    } else if (strcmp(cmd, "search") == 0) {
        apk_search(argc > 2 ? argv[2] : NULL);
    } else if (strcmp(cmd, "upgrade") == 0) {
        printf("Upgrading system packages...\n");
        printf("OK: 0 packages upgraded, 0 packages downgraded\n");
    } else {
        printf("apk: unknown command '%s'. Type 'apk' for help.\n", cmd);
    }
}
