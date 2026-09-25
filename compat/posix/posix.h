#ifndef COMPAT_POSIX_POSIX_H
#define COMPAT_POSIX_POSIX_H

#include <types.h>
#include "stat.h"
#include "fcntl.h"
#include "errno.h"
#include "../../fs/vfs.h"

#define MAX_FD 64

typedef struct {
    vfs_node_t *node;
    uint32_t offset;
    int flags;
    bool in_use;
} posix_file_desc_t;

void posix_init(void);
int posix_open(const char *path, int flags, mode_t mode);
ssize_t posix_read(int fd, void *buf, size_t count);
ssize_t posix_write(int fd, const void *buf, size_t count);
int posix_close(int fd);
off_t posix_lseek(int fd, off_t offset, int whence);
int posix_stat(const char *path, struct posix_stat *buf);
int posix_fstat(int fd, struct posix_stat *buf);
int posix_mkdir(const char *path, mode_t mode);
int posix_unlink(const char *path);
int posix_dup(int oldfd);
int posix_dup2(int oldfd, int newfd);
void *posix_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int posix_munmap(void *addr, size_t length);
void *posix_brk(void *addr);

#endif /* COMPAT_POSIX_POSIX_H */
