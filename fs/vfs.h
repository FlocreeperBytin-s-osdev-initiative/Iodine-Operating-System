#ifndef FS_VFS_H
#define FS_VFS_H

#include <types.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06

#define VFS_NAME_MAX 64
#define VFS_PATH_MAX 256

struct vfs_node;

typedef struct dirent {
    char name[VFS_NAME_MAX];
    uint32_t ino;
    uint32_t type;
    uint32_t size;
} dirent_t;

typedef uint32_t (*vfs_read_type_t)(struct vfs_node *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*vfs_write_type_t)(struct vfs_node *, uint32_t, uint32_t, const uint8_t *);
typedef void (*vfs_open_type_t)(struct vfs_node *);
typedef void (*vfs_close_type_t)(struct vfs_node *);
typedef struct dirent *(*vfs_readdir_type_t)(struct vfs_node *, uint32_t);
typedef struct vfs_node *(*vfs_finddir_type_t)(struct vfs_node *, const char *name);
typedef int (*vfs_mkdir_type_t)(struct vfs_node *, const char *name);
typedef int (*vfs_unlink_type_t)(struct vfs_node *, const char *name);

typedef struct vfs_node {
    char name[VFS_NAME_MAX];
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    uint32_t impl; // Implementation-specific pointer / index

    vfs_read_type_t read;
    vfs_write_type_t write;
    vfs_open_type_t open;
    vfs_close_type_t close;
    vfs_readdir_type_t readdir;
    vfs_finddir_type_t finddir;
    vfs_mkdir_type_t mkdir;
    vfs_unlink_type_t unlink;

    struct vfs_node *ptr; // Alias / mount point forwarding
} vfs_node_t;

void vfs_init(void);
void vfs_mount(const char *path, vfs_node_t *node);
vfs_node_t *vfs_resolve_path(const char *path);

uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buffer);
void vfs_open(vfs_node_t *node);
void vfs_close(vfs_node_t *node);
struct dirent *vfs_readdir(vfs_node_t *node, uint32_t index);
vfs_node_t *vfs_finddir(vfs_node_t *node, const char *name);
int vfs_mkdir(const char *path);
int vfs_create_file(const char *path);
int vfs_unlink(const char *path);

#endif /* FS_VFS_H */
