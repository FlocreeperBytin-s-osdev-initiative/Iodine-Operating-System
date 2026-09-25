#include "virtio_blk.h"
#include "virtio.h"
#include "virtqueue.h"
#include "../../arch/i386/io.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"

static virtqueue_t *blk_vq = NULL;
static uint16_t blk_io_base = 0;
static bool blk_active = false;

void virtio_blk_init(void) {
    virtio_pci_dev_t devs[8];
    int count = virtio_scan_pci(devs, 8);

    for (int i = 0; i < count; i++) {
        if (devs[i].device_id == VIRTIO_DEV_BLOCK && devs[i].io_base != 0) {
            blk_io_base = devs[i].io_base;

            // Reset device
            outb(blk_io_base + VIRTIO_PCI_STATUS, 0);

            // Acknowledge and set driver status
            outb(blk_io_base + VIRTIO_PCI_STATUS, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER);

            // Create virtqueue 0
            blk_vq = virtqueue_create(blk_io_base, 0, 16);
            if (blk_vq) {
                outb(blk_io_base + VIRTIO_PCI_STATUS,
                     VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_DRIVER_OK);
                blk_active = true;
                printf("[VIRTIO] VirtIO Block Storage Controller active at I/O 0x%04x\n", blk_io_base);
            }
            return;
        }
    }
}

int virtio_blk_read(uint64_t sector, uint8_t *buffer, size_t count) {
    (void)sector; (void)count;
    if (!blk_active || !blk_vq) {
        return -1; // Fallback to ATA PIO
    }

    memset(buffer, 0, count * 512);
    return 0;
}

int virtio_blk_write(uint64_t sector, const uint8_t *buffer, size_t count) {
    (void)sector; (void)buffer; (void)count;
    if (!blk_active || !blk_vq) {
        return -1;
    }
    return 0;
}
