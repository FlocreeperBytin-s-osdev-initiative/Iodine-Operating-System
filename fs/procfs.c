#include "procfs.h"
#include "../mm/pmm.h"
#include "../mm/kheap.h"
#include "../drivers/timer.h"
#include "../kernel/process.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"

#define MAX_PROC_ENTRIES 8

typedef struct {
    char name[32];
    void (*generate)(char *buf, size_t max_len);
} proc_entry_t;

static proc_entry_t proc_entries[MAX_PROC_ENTRIES];
static int proc_entry_count = 0;
static vfs_node_t proc_nodes[MAX_PROC_ENTRIES];
static vfs_node_t proc_root;
static dirent_t proc_shared_dirent;

static void gen_version(char *buf, size_t max_len) {
    snprintf(buf, max_len, "Iodine Operating System Kernel v1.0.0 (i386-elf)\nCompiled for x86_32 Protected Mode.\n");
}

static void gen_meminfo(char *buf, size_t max_len) {
    uint32_t total = pmm_get_total_blocks() * PAGE_SIZE;
    uint32_t used  = pmm_get_used_blocks() * PAGE_SIZE;
    uint32_t free  = pmm_get_free_blocks() * PAGE_SIZE;

    size_t heap_used = 0, heap_free = 0;
    kheap_get_stats(&heap_used, &heap_free);

    snprintf(buf, max_len,
             "MemTotal:       %u kB\n"
             "MemFree:        %u kB\n"
             "MemUsed:        %u kB\n"
             "HeapTotal:      %u kB\n"
             "HeapUsed:       %u kB\n"
             "HeapFree:       %u kB\n",
             total / 1024, free / 1024, used / 1024,
             (heap_used + heap_free) / 1024, heap_used / 1024, heap_free / 1024);
}

static void gen_uptime(char *buf, size_t max_len) {
    uint32_t sec = timer_get_uptime_seconds();
    uint32_t ticks = timer_get_ticks();
    snprintf(buf, max_len, "Uptime: %u seconds (%u ticks)\n", sec, ticks);
}

static void gen_cpuinfo(char *buf, size_t max_len) {
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];

    // CPUID function 0: Vendor String
    __asm__ volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(0));
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';

    snprintf(buf, max_len,
             "processor       : 0\n"
             "vendor_id       : %s\n"
             "cpu family      : 6\n"
             "model name      : x86 Compatible Processor\n"
             "stepping        : 1\n"
             "flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov\n",
             vendor);
}

static void gen_tasks(char *buf, size_t max_len) {
    process_info_t procs[16];
    int count = process_list_all(procs, 16);

    int pos = snprintf(buf, max_len, "PID  NAME             STATE    PRIO  TICKS\n");
    for (int i = 0; i < count && pos < (int)max_len - 64; i++) {
        const char *st_str = "READY";
        if (procs[i].state == 1) st_str = "RUNNING";
        else if (procs[i].state == 2) st_str = "SLEEP";
        else if (procs[i].state == 3) st_str = "BLOCKED";
        else if (procs[i].state == 4) st_str = "ZOMBIE";

        pos += snprintf(buf + pos, max_len - pos, "%-4d %-16s %-8s %-5d %u\n",
                        procs[i].pid, procs[i].name, st_str, procs[i].priority, procs[i].cpu_ticks);
    }
}

static uint32_t proc_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    int idx = node->impl;
    if (idx < 0 || idx >= proc_entry_count) return 0;

    char tmp[1024];
    proc_entries[idx].generate(tmp, sizeof(tmp));
    uint32_t len = strlen(tmp);

    if (offset >= len) return 0;
    if (offset + size > len) {
        size = len - offset;
    }

    memcpy(buffer, tmp + offset, size);
    return size;
}

static struct dirent *procfs_readdir(vfs_node_t *node, uint32_t index) {
    (void)node;
    if (index >= (uint32_t)proc_entry_count) return NULL;

    strncpy(proc_shared_dirent.name, proc_nodes[index].name, VFS_NAME_MAX - 1);
    proc_shared_dirent.ino = proc_nodes[index].inode;
    proc_shared_dirent.type = proc_nodes[index].flags;
    proc_shared_dirent.size = 0;
    return &proc_shared_dirent;
}

static vfs_node_t *procfs_finddir(vfs_node_t *node, const char *name) {
    (void)node;
    for (int i = 0; i < proc_entry_count; i++) {
        if (strcmp(proc_nodes[i].name, name) == 0) {
            return &proc_nodes[i];
        }
    }
    return NULL;
}

static void add_proc_file(const char *name, void (*gen)(char *, size_t)) {
    if (proc_entry_count >= MAX_PROC_ENTRIES) return;

    int idx = proc_entry_count;
    strncpy(proc_entries[idx].name, name, 31);
    proc_entries[idx].generate = gen;

    strncpy(proc_nodes[idx].name, name, VFS_NAME_MAX - 1);
    proc_nodes[idx].inode = 2000 + idx;
    proc_nodes[idx].flags = FS_FILE;
    proc_nodes[idx].read = proc_read;
    proc_nodes[idx].impl = idx;

    proc_entry_count++;
}

vfs_node_t *procfs_init(void) {
    memset(&proc_root, 0, sizeof(vfs_node_t));
    strcpy(proc_root.name, "proc");
    proc_root.flags = FS_DIRECTORY;
    proc_root.readdir = procfs_readdir;
    proc_root.finddir = procfs_finddir;

    add_proc_file("version", gen_version);
    add_proc_file("meminfo", gen_meminfo);
    add_proc_file("uptime",  gen_uptime);
    add_proc_file("cpuinfo", gen_cpuinfo);
    add_proc_file("tasks",   gen_tasks);

    return &proc_root;
}
