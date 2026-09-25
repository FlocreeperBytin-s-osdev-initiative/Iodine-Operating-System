#include "devfs.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/keyboard.h"
#include "../drivers/speaker.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"

#define MAX_DEV_NODES 16

static vfs_node_t dev_nodes[MAX_DEV_NODES];
static int dev_node_count = 0;
static vfs_node_t dev_root;
static dirent_t dev_shared_dirent;

// /dev/null
static uint32_t dev_null_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    (void)node; (void)offset; (void)size; (void)buffer;
    return 0; // EOF
}
static uint32_t dev_null_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buffer) {
    (void)node; (void)offset; (void)buffer;
    return size; // Discarded
}

// /dev/zero
static uint32_t dev_zero_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    (void)node; (void)offset;
    memset(buffer, 0, size);
    return size;
}
static uint32_t dev_zero_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buffer) {
    (void)node; (void)offset; (void)buffer;
    return size;
}

// /dev/random
static uint32_t dev_random_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    (void)node; (void)offset;
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = (uint8_t)(rand() & 0xFF);
    }
    return size;
}

// /dev/serial
static uint32_t dev_serial_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buffer) {
    (void)node; (void)offset;
    for (uint32_t i = 0; i < size; i++) {
        serial_putc((char)buffer[i]);
    }
    return size;
}
static uint32_t dev_serial_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    (void)node; (void)offset;
    uint32_t read = 0;
    while (read < size && serial_received()) {
        buffer[read++] = (uint8_t)serial_getc();
    }
    return read;
}

// /dev/vga
static uint32_t dev_vga_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buffer) {
    (void)node; (void)offset;
    for (uint32_t i = 0; i < size; i++) {
        vga_putc((char)buffer[i]);
    }
    return size;
}

// /dev/keyboard
static uint32_t dev_keyboard_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    (void)node; (void)offset;
    uint32_t read = 0;
    while (read < size && keyboard_has_char()) {
        buffer[read++] = (uint8_t)keyboard_getc();
    }
    return read;
}

static struct dirent *devfs_readdir(vfs_node_t *node, uint32_t index) {
    (void)node;
    if (index >= (uint32_t)dev_node_count) {
        return NULL;
    }

    strncpy(dev_shared_dirent.name, dev_nodes[index].name, VFS_NAME_MAX - 1);
    dev_shared_dirent.ino = dev_nodes[index].inode;
    dev_shared_dirent.type = dev_nodes[index].flags;
    dev_shared_dirent.size = 0;
    return &dev_shared_dirent;
}

static vfs_node_t *devfs_finddir(vfs_node_t *node, const char *name) {
    (void)node;
    for (int i = 0; i < dev_node_count; i++) {
        if (strcmp(dev_nodes[i].name, name) == 0) {
            return &dev_nodes[i];
        }
    }
    return NULL;
}

static void register_device(const char *name, vfs_read_type_t read, vfs_write_type_t write) {
    if (dev_node_count >= MAX_DEV_NODES) return;

    vfs_node_t *dev = &dev_nodes[dev_node_count];
    strncpy(dev->name, name, VFS_NAME_MAX - 1);
    dev->inode = 1000 + dev_node_count;
    dev->flags = FS_CHARDEVICE;
    dev->read = read;
    dev->write = write;
    dev_node_count++;
}

vfs_node_t *devfs_init(void) {
    memset(&dev_root, 0, sizeof(vfs_node_t));
    strcpy(dev_root.name, "dev");
    dev_root.flags = FS_DIRECTORY;
    dev_root.readdir = devfs_readdir;
    dev_root.finddir = devfs_finddir;

    register_device("null", dev_null_read, dev_null_write);
    register_device("zero", dev_zero_read, dev_zero_write);
    register_device("random", dev_random_read, dev_null_write);
    register_device("serial", dev_serial_read, dev_serial_write);
    register_device("vga", dev_null_read, dev_vga_write);
    register_device("keyboard", dev_keyboard_read, dev_null_write);

    return &dev_root;
}
