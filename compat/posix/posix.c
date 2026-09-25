#include "posix.h"
#include "../../mm/kheap.h"
#include "../../lib/string.h"
#include "../../lib/stdio.h"
#include "../../drivers/timer.h"

int errno = 0;

static posix_file_desc_t fd_table[MAX_FD];
static uintptr_t current_brk = 0x03000000;

void posix_init(void) {
    memset(fd_table, 0, sizeof(fd_table));

    // fd 0: stdin (/dev/keyboard)
    vfs_node_t *kbd = vfs_resolve_path("/dev/keyboard");
    if (kbd) {
        fd_table[0].node = kbd;
        fd_table[0].flags = O_RDONLY;
        fd_table[0].in_use = true;
    }

    // fd 1: stdout (/dev/vga)
    vfs_node_t *vga = vfs_resolve_path("/dev/vga");
    if (vga) {
        fd_table[1].node = vga;
        fd_table[1].flags = O_WRONLY;
        fd_table[1].in_use = true;
    }

    // fd 2: stderr (/dev/vga)
    if (vga) {
        fd_table[2].node = vga;
        fd_table[2].flags = O_WRONLY;
        fd_table[2].in_use = true;
    }
}

static int find_free_fd(void) {
    for (int i = 3; i < MAX_FD; i++) {
        if (!fd_table[i].in_use) {
            return i;
        }
    }
    return -1;
}

int posix_open(const char *path, int flags, mode_t mode) {
    (void)mode;
    if (!path) {
        errno = EFAULT;
        return -1;
    }

    vfs_node_t *node = vfs_resolve_path(path);
    if (!node) {
        if (flags & O_CREAT) {
            if (vfs_create_file(path) != 0) {
                errno = ENOENT;
                return -1;
            }
            node = vfs_resolve_path(path);
        } else {
            errno = ENOENT;
            return -1;
        }
    }

    if (!node) {
        errno = ENOENT;
        return -1;
    }

    int fd = find_free_fd();
    if (fd == -1) {
        errno = EMFILE;
        return -1;
    }

    fd_table[fd].node = node;
    fd_table[fd].flags = flags;
    fd_table[fd].offset = 0;
    fd_table[fd].in_use = true;

    if (flags & O_TRUNC) {
        node->length = 0;
    }
    if (flags & O_APPEND) {
        fd_table[fd].offset = node->length;
    }

    vfs_open(node);
    return fd;
}

ssize_t posix_read(int fd, void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].in_use || !fd_table[fd].node) {
        errno = EBADF;
        return -1;
    }
    if (!buf) {
        errno = EFAULT;
        return -1;
    }

    posix_file_desc_t *desc = &fd_table[fd];
    uint32_t bytes = vfs_read(desc->node, desc->offset, count, (uint8_t *)buf);
    desc->offset += bytes;
    return (ssize_t)bytes;
}

ssize_t posix_write(int fd, const void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].in_use || !fd_table[fd].node) {
        errno = EBADF;
        return -1;
    }
    if (!buf) {
        errno = EFAULT;
        return -1;
    }

    posix_file_desc_t *desc = &fd_table[fd];
    uint32_t bytes = vfs_write(desc->node, desc->offset, count, (const uint8_t *)buf);
    desc->offset += bytes;
    return (ssize_t)bytes;
}

int posix_close(int fd) {
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].in_use) {
        errno = EBADF;
        return -1;
    }

    if (fd_table[fd].node) {
        vfs_close(fd_table[fd].node);
    }
    fd_table[fd].node = NULL;
    fd_table[fd].in_use = false;
    fd_table[fd].offset = 0;
    fd_table[fd].flags = 0;
    return 0;
}

off_t posix_lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].in_use || !fd_table[fd].node) {
        errno = EBADF;
        return -1;
    }

    posix_file_desc_t *desc = &fd_table[fd];
    off_t new_off = desc->offset;

    if (whence == SEEK_SET) {
        new_off = offset;
    } else if (whence == SEEK_CUR) {
        new_off += offset;
    } else if (whence == SEEK_END) {
        new_off = (off_t)desc->node->length + offset;
    } else {
        errno = EINVAL;
        return -1;
    }

    if (new_off < 0) {
        errno = EINVAL;
        return -1;
    }

    desc->offset = (uint32_t)new_off;
    return new_off;
}

int posix_stat(const char *path, struct posix_stat *buf) {
    if (!path || !buf) {
        errno = EFAULT;
        return -1;
    }

    vfs_node_t *node = vfs_resolve_path(path);
    if (!node) {
        errno = ENOENT;
        return -1;
    }

    memset(buf, 0, sizeof(struct posix_stat));
    buf->st_ino = node->inode;
    buf->st_size = node->length;
    buf->st_blksize = 4096;
    buf->st_blocks = (node->length + 511) / 512;
    buf->st_nlink = 1;

    if (node->flags & FS_DIRECTORY) {
        buf->st_mode = S_IFDIR | 0755;
    } else if (node->flags & FS_CHARDEVICE) {
        buf->st_mode = S_IFCHR | 0666;
    } else {
        buf->st_mode = S_IFREG | 0644;
    }

    buf->st_mtime = timer_get_uptime_seconds();
    buf->st_atime = buf->st_mtime;
    buf->st_ctime = buf->st_mtime;
    return 0;
}

int posix_fstat(int fd, struct posix_stat *buf) {
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].in_use || !fd_table[fd].node) {
        errno = EBADF;
        return -1;
    }
    if (!buf) {
        errno = EFAULT;
        return -1;
    }

    vfs_node_t *node = fd_table[fd].node;
    memset(buf, 0, sizeof(struct posix_stat));
    buf->st_ino = node->inode;
    buf->st_size = node->length;
    buf->st_blksize = 4096;
    buf->st_blocks = (node->length + 511) / 512;
    buf->st_nlink = 1;

    if (node->flags & FS_DIRECTORY) {
        buf->st_mode = S_IFDIR | 0755;
    } else if (node->flags & FS_CHARDEVICE) {
        buf->st_mode = S_IFCHR | 0666;
    } else {
        buf->st_mode = S_IFREG | 0644;
    }

    buf->st_mtime = timer_get_uptime_seconds();
    buf->st_atime = buf->st_mtime;
    buf->st_ctime = buf->st_mtime;
    return 0;
}

int posix_mkdir(const char *path, mode_t mode) {
    (void)mode;
    if (!path) {
        errno = EFAULT;
        return -1;
    }
    if (vfs_mkdir(path) != 0) {
        errno = EEXIST;
        return -1;
    }
    return 0;
}

int posix_unlink(const char *path) {
    if (!path) {
        errno = EFAULT;
        return -1;
    }
    if (vfs_unlink(path) != 0) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}

int posix_dup(int oldfd) {
    if (oldfd < 0 || oldfd >= MAX_FD || !fd_table[oldfd].in_use) {
        errno = EBADF;
        return -1;
    }

    int newfd = find_free_fd();
    if (newfd == -1) {
        errno = EMFILE;
        return -1;
    }

    fd_table[newfd] = fd_table[oldfd];
    return newfd;
}

int posix_dup2(int oldfd, int newfd) {
    if (oldfd < 0 || oldfd >= MAX_FD || !fd_table[oldfd].in_use || newfd < 0 || newfd >= MAX_FD) {
        errno = EBADF;
        return -1;
    }

    if (oldfd == newfd) {
        return newfd;
    }

    if (fd_table[newfd].in_use) {
        posix_close(newfd);
    }

    fd_table[newfd] = fd_table[oldfd];
    return newfd;
}

void *posix_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    (void)addr; (void)prot; (void)flags; (void)offset;
    void *ptr = kmalloc(length);
    if (!ptr) {
        errno = ENOMEM;
        return (void *)-1;
    }

    if (fd >= 0 && fd < MAX_FD && fd_table[fd].in_use && fd_table[fd].node) {
        vfs_read(fd_table[fd].node, offset, length, (uint8_t *)ptr);
    } else {
        memset(ptr, 0, length);
    }

    return ptr;
}

int posix_munmap(void *addr, size_t length) {
    (void)length;
    if (addr && addr != (void *)-1) {
        kfree(addr);
    }
    return 0;
}

void *posix_brk(void *addr) {
    if (!addr || (uintptr_t)addr < 0x03000000) {
        return (void *)current_brk;
    }

    uintptr_t new_brk = (uintptr_t)addr;
    if (new_brk > 0x08000000) { // Max 128MB brk boundary
        errno = ENOMEM;
        return (void *)current_brk;
    }

    current_brk = new_brk;
    return (void *)current_brk;
}
