#include "vfs.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"

#define MAX_MOUNTS 16

typedef struct {
    char path[VFS_PATH_MAX];
    vfs_node_t *node;
} mount_entry_t;

static mount_entry_t mount_table[MAX_MOUNTS];
static int mount_count = 0;
static vfs_node_t *vfs_root = NULL;

void vfs_init(void) {
    memset(mount_table, 0, sizeof(mount_table));
    mount_count = 0;
    vfs_root = NULL;
}

void vfs_mount(const char *path, vfs_node_t *node) {
    if (!path || !node || mount_count >= MAX_MOUNTS) return;

    if (strcmp(path, "/") == 0) {
        vfs_root = node;
    }

    strncpy(mount_table[mount_count].path, path, VFS_PATH_MAX - 1);
    mount_table[mount_count].node = node;
    mount_count++;
}

vfs_node_t *vfs_resolve_path(const char *path) {
    if (!path || !vfs_root) return NULL;

    // Check mount table for exact match first
    for (int i = 0; i < mount_count; i++) {
        if (strcmp(mount_table[i].path, path) == 0) {
            return mount_table[i].node;
        }
    }

    // Check if path starts with a mount point (e.g. /dev/...)
    for (int i = 0; i < mount_count; i++) {
        if (strcmp(mount_table[i].path, "/") == 0) continue;
        size_t mlen = strlen(mount_table[i].path);
        if (strncmp(mount_table[i].path, path, mlen) == 0) {
            if (path[mlen] == '/' || path[mlen] == '\0') {
                const char *subpath = path + mlen;
                if (*subpath == '/') subpath++;
                if (*subpath == '\0') return mount_table[i].node;
                return vfs_finddir(mount_table[i].node, subpath);
            }
        }
    }

    // Traverse from root
    vfs_node_t *curr = vfs_root;
    char path_copy[VFS_PATH_MAX];
    strncpy(path_copy, path, VFS_PATH_MAX - 1);
    path_copy[VFS_PATH_MAX - 1] = '\0';

    char *token = strtok(path_copy, "/");
    while (token) {
        if (strcmp(token, ".") == 0) {
            // Stay in current dir
        } else if (strcmp(token, "..") == 0) {
            vfs_node_t *parent = vfs_finddir(curr, "..");
            if (parent) curr = parent;
        } else {
            vfs_node_t *next = vfs_finddir(curr, token);
            if (!next) return NULL;
            curr = next;
        }
        token = strtok(NULL, "/");
    }

    return curr;
}

uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (node && node->read) {
        return node->read(node, offset, size, buffer);
    }
    return 0;
}

uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buffer) {
    if (node && node->write) {
        return node->write(node, offset, size, buffer);
    }
    return 0;
}

void vfs_open(vfs_node_t *node) {
    if (node && node->open) {
        node->open(node);
    }
}

void vfs_close(vfs_node_t *node) {
    if (node && node->close) {
        node->close(node);
    }
}

struct dirent *vfs_readdir(vfs_node_t *node, uint32_t index) {
    if (node && (node->flags & FS_DIRECTORY) && node->readdir) {
        return node->readdir(node, index);
    }
    return NULL;
}

vfs_node_t *vfs_finddir(vfs_node_t *node, const char *name) {
    if (node && (node->flags & FS_DIRECTORY) && node->finddir) {
        return node->finddir(node, name);
    }
    return NULL;
}

int vfs_mkdir(const char *path) {
    if (!path) return -1;

    char dir_path[VFS_PATH_MAX];
    char base_name[VFS_NAME_MAX];
    strncpy(dir_path, path, VFS_PATH_MAX - 1);

    char *last_slash = strrchr(dir_path, '/');
    if (!last_slash) {
        // Current directory
        return -1;
    }

    *last_slash = '\0';
    strcpy(base_name, last_slash + 1);

    const char *parent_path = (last_slash == dir_path) ? "/" : dir_path;
    vfs_node_t *parent = vfs_resolve_path(parent_path);
    if (!parent || !(parent->flags & FS_DIRECTORY) || !parent->mkdir) {
        return -1;
    }

    return parent->mkdir(parent, base_name);
}

int vfs_create_file(const char *path) {
    if (!path) return -1;

    char dir_path[VFS_PATH_MAX];
    char base_name[VFS_NAME_MAX];
    strncpy(dir_path, path, VFS_PATH_MAX - 1);

    char *last_slash = strrchr(dir_path, '/');
    if (!last_slash) {
        return -1;
    }

    *last_slash = '\0';
    strcpy(base_name, last_slash + 1);

    const char *parent_path = (last_slash == dir_path) ? "/" : dir_path;
    vfs_node_t *parent = vfs_resolve_path(parent_path);
    if (!parent || !(parent->flags & FS_DIRECTORY)) {
        return -1;
    }

    // Check if it already exists
    if (vfs_finddir(parent, base_name)) {
        return 0; // Exists already
    }

    extern vfs_node_t *ramfs_create_file_node(const char *name, vfs_node_t *parent);
    vfs_node_t *new_node = ramfs_create_file_node(base_name, parent);
    return new_node ? 0 : -1;
}

int vfs_unlink(const char *path) {
    if (!path) return -1;

    char dir_path[VFS_PATH_MAX];
    char base_name[VFS_NAME_MAX];
    strncpy(dir_path, path, VFS_PATH_MAX - 1);

    char *last_slash = strrchr(dir_path, '/');
    if (!last_slash) return -1;

    *last_slash = '\0';
    strcpy(base_name, last_slash + 1);

    const char *parent_path = (last_slash == dir_path) ? "/" : dir_path;
    vfs_node_t *parent = vfs_resolve_path(parent_path);
    if (!parent || !(parent->flags & FS_DIRECTORY) || !parent->unlink) {
        return -1;
    }

    return parent->unlink(parent, base_name);
}
