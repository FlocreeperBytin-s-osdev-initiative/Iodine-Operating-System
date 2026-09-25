#ifndef FS_RAMFS_H
#define FS_RAMFS_H

#include "vfs.h"

vfs_node_t *ramfs_init(void);
vfs_node_t *ramfs_create_file(vfs_node_t *parent, const char *name, const char *content);
vfs_node_t *ramfs_create_dir(vfs_node_t *parent, const char *name);
vfs_node_t *ramfs_create_file_node(const char *name, vfs_node_t *parent);

#endif /* FS_RAMFS_H */
