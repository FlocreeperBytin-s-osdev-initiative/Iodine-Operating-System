#include "ramfs.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"
#include "../lib/stdio.h"

#define MAX_DIR_CHILDREN 64

typedef struct ramfs_entry {
    vfs_node_t node;
    uint8_t *data;
    uint32_t capacity;

    // For directories
    struct ramfs_entry *children[MAX_DIR_CHILDREN];
    uint32_t num_children;
    struct ramfs_entry *parent;
} ramfs_entry_t;

static dirent_t shared_dirent;
static uint32_t next_inode = 1;

static uint32_t ramfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    ramfs_entry_t *entry = (ramfs_entry_t *)node->impl;
    if (!entry || !entry->data || offset >= node->length) {
        return 0;
    }

    if (offset + size > node->length) {
        size = node->length - offset;
    }

    memcpy(buffer, entry->data + offset, size);
    return size;
}

static uint32_t ramfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buffer) {
    ramfs_entry_t *entry = (ramfs_entry_t *)node->impl;
    if (!entry) return 0;

    uint32_t needed = offset + size;
    if (needed > entry->capacity) {
        uint32_t new_cap = needed * 2;
        if (new_cap < 64) new_cap = 64;
        uint8_t *new_data = (uint8_t *)krealloc(entry->data, new_cap);
        if (!new_data) return 0;
        entry->data = new_data;
        entry->capacity = new_cap;
    }

    memcpy(entry->data + offset, buffer, size);
    if (needed > node->length) {
        node->length = needed;
    }
    return size;
}

static struct dirent *ramfs_readdir(vfs_node_t *node, uint32_t index) {
    ramfs_entry_t *entry = (ramfs_entry_t *)node->impl;
    if (!entry || !(node->flags & FS_DIRECTORY)) return NULL;

    if (index >= entry->num_children) {
        return NULL;
    }

    ramfs_entry_t *child = entry->children[index];
    strncpy(shared_dirent.name, child->node.name, VFS_NAME_MAX - 1);
    shared_dirent.ino = child->node.inode;
    shared_dirent.type = child->node.flags;
    shared_dirent.size = child->node.length;

    return &shared_dirent;
}

static vfs_node_t *ramfs_finddir(vfs_node_t *node, const char *name) {
    ramfs_entry_t *entry = (ramfs_entry_t *)node->impl;
    if (!entry || !(node->flags & FS_DIRECTORY)) return NULL;

    if (strcmp(name, ".") == 0) {
        return node;
    }
    if (strcmp(name, "..") == 0) {
        if (entry->parent) return &entry->parent->node;
        return node; // Root parent is root
    }

    for (uint32_t i = 0; i < entry->num_children; i++) {
        if (strcmp(entry->children[i]->node.name, name) == 0) {
            return &entry->children[i]->node;
        }
    }

    return NULL;
}

static int ramfs_mkdir(vfs_node_t *parent_node, const char *name) {
    vfs_node_t *new_dir = ramfs_create_dir(parent_node, name);
    return new_dir ? 0 : -1;
}

static int ramfs_unlink(vfs_node_t *parent_node, const char *name) {
    ramfs_entry_t *parent_entry = (ramfs_entry_t *)parent_node->impl;
    if (!parent_entry) return -1;

    for (uint32_t i = 0; i < parent_entry->num_children; i++) {
        if (strcmp(parent_entry->children[i]->node.name, name) == 0) {
            ramfs_entry_t *child = parent_entry->children[i];
            if (child->data) {
                kfree(child->data);
            }
            kfree(child);

            // Shift remaining children left
            for (uint32_t j = i; j < parent_entry->num_children - 1; j++) {
                parent_entry->children[j] = parent_entry->children[j + 1];
            }
            parent_entry->num_children--;
            return 0;
        }
    }
    return -1;
}

vfs_node_t *ramfs_create_dir(vfs_node_t *parent, const char *name) {
    ramfs_entry_t *entry = (ramfs_entry_t *)kcalloc(1, sizeof(ramfs_entry_t));
    if (!entry) return NULL;

    strncpy(entry->node.name, name, VFS_NAME_MAX - 1);
    entry->node.inode = next_inode++;
    entry->node.flags = FS_DIRECTORY;
    entry->node.length = 0;
    entry->node.impl = (uint32_t)entry;
    entry->node.readdir = ramfs_readdir;
    entry->node.finddir = ramfs_finddir;
    entry->node.mkdir = ramfs_mkdir;
    entry->node.unlink = ramfs_unlink;

    if (parent) {
        ramfs_entry_t *p_entry = (ramfs_entry_t *)parent->impl;
        if (p_entry && p_entry->num_children < MAX_DIR_CHILDREN) {
            entry->parent = p_entry;
            p_entry->children[p_entry->num_children++] = entry;
        }
    }

    return &entry->node;
}

vfs_node_t *ramfs_create_file(vfs_node_t *parent, const char *name, const char *content) {
    ramfs_entry_t *entry = (ramfs_entry_t *)kcalloc(1, sizeof(ramfs_entry_t));
    if (!entry) return NULL;

    strncpy(entry->node.name, name, VFS_NAME_MAX - 1);
    entry->node.inode = next_inode++;
    entry->node.flags = FS_FILE;
    entry->node.impl = (uint32_t)entry;
    entry->node.read = ramfs_read;
    entry->node.write = ramfs_write;

    if (content) {
        uint32_t len = strlen(content);
        entry->data = (uint8_t *)kmalloc(len + 1);
        if (entry->data) {
            memcpy(entry->data, content, len + 1);
            entry->capacity = len + 1;
            entry->node.length = len;
        }
    }

    if (parent) {
        ramfs_entry_t *p_entry = (ramfs_entry_t *)parent->impl;
        if (p_entry && p_entry->num_children < MAX_DIR_CHILDREN) {
            entry->parent = p_entry;
            p_entry->children[p_entry->num_children++] = entry;
        }
    }

    return &entry->node;
}

vfs_node_t *ramfs_create_file_node(const char *name, vfs_node_t *parent) {
    return ramfs_create_file(parent, name, "");
}

vfs_node_t *ramfs_init(void) {
    vfs_node_t *root = ramfs_create_dir(NULL, "/");

    // Create system directories
    vfs_node_t *etc  = ramfs_create_dir(root, "etc");
    vfs_node_t *home = ramfs_create_dir(root, "home");
    vfs_node_t *user = ramfs_create_dir(home, "user");
    vfs_node_t *bin  = ramfs_create_dir(root, "bin");
    vfs_node_t *docs = ramfs_create_dir(root, "docs");
    vfs_node_t *tmp  = ramfs_create_dir(root, "tmp");
    (void)tmp;

    // System files
    ramfs_create_file(root, "README.txt",
        "=====================================================\n"
        "             WELCOME TO IODINE OS v1.0.0             \n"
        "=====================================================\n"
        "Iodine OS is a 100% custom, standalone 32-bit x86 OS\n"
        "built entirely from bare metal.\n\n"
        "- NOT Linux, NOT BSD, NO foreign distro.\n"
        "- Preemptive multitasking round-robin scheduler.\n"
        "- Virtual File System (VFS) with RamFS, DevFS, ProcFS.\n"
        "- Dynamic Memory Management (PMM, VMM paging, Heap).\n"
        "- Interactive shell 'ish' with full line editor.\n"
        "- Built-in games and applications: Snake, Nano Editor,\n"
        "  Matrix digital rain, Calculator, Melodies, Top.\n\n"
        "Type 'help' in the shell to see all available commands!\n");

    ramfs_create_file(etc, "hostname", "iodine-os\n");
    ramfs_create_file(etc, "version", "Iodine Operating System 1.0.0 (Release 2026)\n");
    ramfs_create_file(etc, "motd",
        "Welcome to Iodine OS!\n"
        "Protected Mode Kernel running smoothly.\n"
        "Type 'help' for command reference or 'snake' to play!\n");
    ramfs_create_file(etc, "sysinfo",
        "OS: Iodine OS (x86_32 Protected Mode)\n"
        "Kernel: Monolithic Iodine Kernel v1.0.0\n"
        "Architecture: Intel i386 / IA-32\n"
        "Shell: Iodine Shell (ish)\n"
        "Memory Model: 4GB Flat Paged Virtual Memory\n");

    ramfs_create_file(user, "welcome.txt",
        "Hello User!\n"
        "You are logged into Iodine OS as root.\n"
        "Try running these commands:\n"
        "  fetch     - Display ASCII art system banner\n"
        "  snake     - Play the classic Snake game\n"
        "  matrix    - Show digital matrix green rain\n"
        "  edit test - Open the visual full-screen editor\n"
        "  top       - Real-time dynamic system monitor\n"
        "  calc      - Arithmetic expression calculator\n"
        "  date      - Check real hardware RTC time\n"
        "  mem       - Physical and heap memory usage\n"
        "  ps        - View active processes\n");

    ramfs_create_file(user, "notes.txt",
        "- Iodine OS build complete!\n"
        "- Zero mistakes, 100% custom bare metal architecture.\n"
        "- Have fun exploring!\n");

    ramfs_create_file(docs, "commands.txt",
        "IODINE OS COMMAND MANUAL:\n"
        "-------------------------\n"
        "help [cmd]         - Command reference\n"
        "clear              - Clear display\n"
        "echo [args...]     - Output text\n"
        "ls [-l] [dir]      - Directory listing\n"
        "cd [path]          - Change directory\n"
        "pwd                - Current working directory\n"
        "cat [file]         - Print file contents\n"
        "touch [file]       - Create empty file\n"
        "mkdir [path]       - Make directory\n"
        "rm [file]          - Delete file or directory\n"
        "write [f] [text]   - Write line into file\n"
        "edit [file]        - Nano-like text editor\n"
        "uname [-a]         - Kernel and OS name\n"
        "date               - Hardware RTC date & time\n"
        "uptime             - System uptime\n"
        "mem / free         - RAM & heap statistics\n"
        "ps                 - Process list\n"
        "top                - Live interactive monitor\n"
        "kill [pid]         - Terminate process\n"
        "sleep [sec]        - Pause execution\n"
        "calc [expression]  - Math expression solver\n"
        "beep [freq] [ms]   - PC speaker sound\n"
        "melody             - Play 8-bit fanfare tune\n"
        "color [fg] [bg]    - Change terminal palette\n"
        "hexdump [file]     - Hexadecimal file inspection\n"
        "find [name]        - File search\n"
        "wc [file]          - Line, word, byte counter\n"
        "snake              - Arcade Snake game\n"
        "matrix             - Matrix rain effect\n"
        "fetch              - Neofetch-style system info\n"
        "reboot             - Reboot machine\n"
        "shutdown           - Power off machine\n");

    // Virtual binary tags
    ramfs_create_file(bin, "sh", "[built-in shell]\n");
    ramfs_create_file(bin, "edit", "[visual editor]\n");
    ramfs_create_file(bin, "snake", "[arcade snake game]\n");
    ramfs_create_file(bin, "matrix", "[matrix rain effect]\n");
    ramfs_create_file(bin, "top", "[system monitor]\n");
    ramfs_create_file(bin, "calc", "[math calculator]\n");

    return root;
}
