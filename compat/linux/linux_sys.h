#ifndef COMPAT_LINUX_SYS_H
#define COMPAT_LINUX_SYS_H

#include <types.h>

/* Linux x86_64 / x86 System Call Numbers */
#define LINUX_SYS_READ             0
#define LINUX_SYS_WRITE            1
#define LINUX_SYS_OPEN             2
#define LINUX_SYS_CLOSE            3
#define LINUX_SYS_STAT             4
#define LINUX_SYS_FSTAT            5
#define LINUX_SYS_LSEEK            8
#define LINUX_SYS_MMAP             9
#define LINUX_SYS_MPROTECT        10
#define LINUX_SYS_MUNMAP          11
#define LINUX_SYS_BRK             12
#define LINUX_SYS_RT_SIGACTION    13
#define LINUX_SYS_RT_SIGPROCMASK  14
#define LINUX_SYS_IOCTL           16
#define LINUX_SYS_WRITEV          20
#define LINUX_SYS_ACCESS          21
#define LINUX_SYS_PIPE            22
#define LINUX_SYS_SCHED_YIELD     24
#define LINUX_SYS_DUP             32
#define LINUX_SYS_DUP2            33
#define LINUX_SYS_GETPID          39
#define LINUX_SYS_FORK            57
#define LINUX_SYS_EXECVE          59
#define LINUX_SYS_EXIT            60
#define LINUX_SYS_WAIT4           61
#define LINUX_SYS_KILL            62
#define LINUX_SYS_UNAME           63
#define LINUX_SYS_FCNTL           72
#define LINUX_SYS_GETCWD          79
#define LINUX_SYS_CHDIR           80
#define LINUX_SYS_MKDIR           83
#define LINUX_SYS_RMDIR           84
#define LINUX_SYS_UNLINK          87
#define LINUX_SYS_READLINK        89
#define LINUX_SYS_GETTIMEOFDAY    96
#define LINUX_SYS_GETUID         102
#define LINUX_SYS_GETGID         104
#define LINUX_SYS_GETEUID        107
#define LINUX_SYS_GETEGID        108
#define LINUX_SYS_GETDENTS64     217
#define LINUX_SYS_CLOCK_GETTIME  228
#define LINUX_SYS_EXIT_GROUP     231

/* Linux Structure Layouts */
struct linux_timespec {
    int64_t tv_sec;
    int64_t tv_nsec;
};

struct linux_timeval {
    int64_t tv_sec;
    int64_t tv_usec;
};

struct linux_timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

struct linux_stat64 {
    uint64_t st_dev;
    uint64_t st_ino;
    uint64_t st_nlink;
    uint32_t st_mode;
    uint32_t st_uid;
    uint32_t st_gid;
    uint32_t __pad0;
    uint64_t st_rdev;
    int64_t  st_size;
    int64_t  st_blksize;
    int64_t  st_blocks;
    uint64_t st_atime;
    uint64_t st_atime_nsec;
    uint64_t st_mtime;
    uint64_t st_mtime_nsec;
    uint64_t st_ctime;
    uint64_t st_ctime_nsec;
    int64_t  __unused[3];
};

struct linux_utsname {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char domainname[65];
};

struct linux_dirent64 {
    uint64_t d_ino;
    int64_t  d_off;
    uint16_t d_reclen;
    uint8_t  d_type;
    char     d_name[];
};

struct linux_iovec {
    void  *iov_base;
    size_t iov_len;
};

void linux_translator_init(void);
int64_t linux_syscall_translate(int64_t sys_no, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5, int64_t arg6);

#endif /* COMPAT_LINUX_SYS_H */
