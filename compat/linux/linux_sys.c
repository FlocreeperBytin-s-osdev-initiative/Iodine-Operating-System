#include "linux_sys.h"
#include "../posix/posix.h"
#include "../posix/errno.h"
#include "../../kernel/process.h"
#include "../../drivers/timer.h"
#include "../../drivers/rtc.h"
#include "../../shell/commands.h"
#include "../../lib/string.h"
#include "../../lib/stdio.h"

void linux_translator_init(void) {
    posix_init();
}

int64_t linux_syscall_translate(int64_t sys_no, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5, int64_t arg6) {
    (void)arg4; (void)arg5; (void)arg6;

    switch (sys_no) {
        case LINUX_SYS_READ: {
            ssize_t ret = posix_read((int)arg1, (void *)(uintptr_t)arg2, (size_t)arg3);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_WRITE: {
            ssize_t ret = posix_write((int)arg1, (const void *)(uintptr_t)arg2, (size_t)arg3);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_OPEN: {
            int ret = posix_open((const char *)(uintptr_t)arg1, (int)arg2, (mode_t)arg3);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_CLOSE: {
            int ret = posix_close((int)arg1);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_STAT: {
            struct posix_stat pst;
            int ret = posix_stat((const char *)(uintptr_t)arg1, &pst);
            if (ret < 0) return -errno;

            struct linux_stat64 *lst = (struct linux_stat64 *)(uintptr_t)arg2;
            if (!lst) return -EFAULT;

            memset(lst, 0, sizeof(struct linux_stat64));
            lst->st_ino = pst.st_ino;
            lst->st_mode = pst.st_mode;
            lst->st_nlink = pst.st_nlink;
            lst->st_size = pst.st_size;
            lst->st_blksize = pst.st_blksize;
            lst->st_blocks = pst.st_blocks;
            lst->st_atime = pst.st_atime;
            lst->st_mtime = pst.st_mtime;
            lst->st_ctime = pst.st_ctime;
            return 0;
        }

        case LINUX_SYS_FSTAT: {
            struct posix_stat pst;
            int ret = posix_fstat((int)arg1, &pst);
            if (ret < 0) return -errno;

            struct linux_stat64 *lst = (struct linux_stat64 *)(uintptr_t)arg2;
            if (!lst) return -EFAULT;

            memset(lst, 0, sizeof(struct linux_stat64));
            lst->st_ino = pst.st_ino;
            lst->st_mode = pst.st_mode;
            lst->st_nlink = pst.st_nlink;
            lst->st_size = pst.st_size;
            lst->st_blksize = pst.st_blksize;
            lst->st_blocks = pst.st_blocks;
            lst->st_atime = pst.st_atime;
            lst->st_mtime = pst.st_mtime;
            lst->st_ctime = pst.st_ctime;
            return 0;
        }

        case LINUX_SYS_LSEEK: {
            off_t ret = posix_lseek((int)arg1, (off_t)arg2, (int)arg3);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_MMAP: {
            void *ret = posix_mmap((void *)(uintptr_t)arg1, (size_t)arg2, (int)arg3, (int)arg4, (int)arg5, (off_t)arg6);
            if (ret == (void *)-1) return -errno;
            return (int64_t)(uintptr_t)ret;
        }

        case LINUX_SYS_MPROTECT:
            return 0; // Succeed silently

        case LINUX_SYS_MUNMAP: {
            int ret = posix_munmap((void *)(uintptr_t)arg1, (size_t)arg2);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_BRK: {
            void *ret = posix_brk((void *)(uintptr_t)arg1);
            return (int64_t)(uintptr_t)ret;
        }

        case LINUX_SYS_WRITEV: {
            int fd = (int)arg1;
            const struct linux_iovec *iov = (const struct linux_iovec *)(uintptr_t)arg2;
            int count = (int)arg3;
            if (!iov || count < 0) return -EFAULT;

            ssize_t total = 0;
            for (int i = 0; i < count; i++) {
                ssize_t wr = posix_write(fd, iov[i].iov_base, iov[i].iov_len);
                if (wr < 0) return (total > 0) ? total : -errno;
                total += wr;
            }
            return total;
        }

        case LINUX_SYS_ACCESS: {
            struct posix_stat pst;
            int ret = posix_stat((const char *)(uintptr_t)arg1, &pst);
            return (ret < 0) ? -errno : 0;
        }

        case LINUX_SYS_DUP: {
            int ret = posix_dup((int)arg1);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_DUP2: {
            int ret = posix_dup2((int)arg1, (int)arg2);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_GETPID: {
            process_t *curr = process_get_current();
            return curr ? curr->pid : 1;
        }

        case LINUX_SYS_FORK:
            return 100; // Simulated child PID

        case LINUX_SYS_EXIT:
        case LINUX_SYS_EXIT_GROUP:
            process_exit((int)arg1);
            return 0;

        case LINUX_SYS_UNAME: {
            struct linux_utsname *un = (struct linux_utsname *)(uintptr_t)arg1;
            if (!un) return -EFAULT;

            strncpy(un->sysname, "Linux", 64);
            strncpy(un->nodename, "iodine-os", 64);
            strncpy(un->release, "6.6.0-iodine", 64);
            strncpy(un->version, "#1 Iodine Monolithic Kernel 2026", 64);
            strncpy(un->machine, "x86_64", 64);
            strncpy(un->domainname, "localdomain", 64);
            return 0;
        }

        case LINUX_SYS_GETCWD: {
            char *buf = (char *)(uintptr_t)arg1;
            size_t size = (size_t)arg2;
            if (!buf) return -EFAULT;

            const char *cwd = shell_get_cwd();
            size_t len = strlen(cwd) + 1;
            if (size < len) return -ERANGE;

            memcpy(buf, cwd, len);
            return (int64_t)(uintptr_t)buf;
        }

        case LINUX_SYS_CHDIR: {
            const char *path = (const char *)(uintptr_t)arg1;
            if (!path) return -EFAULT;

            vfs_node_t *node = vfs_resolve_path(path);
            if (!node) return -ENOENT;
            if (!(node->flags & FS_DIRECTORY)) return -ENOTDIR;

            shell_set_cwd(path);
            return 0;
        }

        case LINUX_SYS_MKDIR: {
            int ret = posix_mkdir((const char *)(uintptr_t)arg1, (mode_t)arg2);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_UNLINK:
        case LINUX_SYS_RMDIR: {
            int ret = posix_unlink((const char *)(uintptr_t)arg1);
            return (ret < 0) ? -errno : ret;
        }

        case LINUX_SYS_GETTIMEOFDAY: {
            struct linux_timeval *tv = (struct linux_timeval *)(uintptr_t)arg1;
            if (tv) {
                tv->tv_sec = timer_get_uptime_seconds();
                tv->tv_usec = (timer_get_uptime_ms() % 1000) * 1000;
            }
            return 0;
        }

        case LINUX_SYS_CLOCK_GETTIME: {
            struct linux_timespec *tp = (struct linux_timespec *)(uintptr_t)arg2;
            if (!tp) return -EFAULT;
            tp->tv_sec = timer_get_uptime_seconds();
            tp->tv_nsec = (timer_get_uptime_ms() % 1000) * 1000000;
            return 0;
        }

        case LINUX_SYS_GETUID:
        case LINUX_SYS_GETGID:
        case LINUX_SYS_GETEUID:
        case LINUX_SYS_GETEGID:
            return 0; // Root user UID/GID 0

        case LINUX_SYS_IOCTL:
            return 0; // Succeed basic terminal ioctl

        case LINUX_SYS_SCHED_YIELD:
            process_yield();
            return 0;

        default:
            return -ENOSYS; // Not implemented
    }
}
