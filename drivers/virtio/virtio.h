#ifndef DRIVERS_VIRTIO_H
#define DRIVERS_VIRTIO_H

#include <types.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define VIRTIO_VENDOR_ID   0x1AF4

/* VirtIO Device IDs */
#define VIRTIO_DEV_NET     0x1000
#define VIRTIO_DEV_BLOCK   0x1001
#define VIRTIO_DEV_CONSOLE 0x1003
#define VIRTIO_DEV_RNG     0x1004
#define VIRTIO_DEV_BALLOON 0x1005
#define VIRTIO_DEV_9P      0x1009
#define VIRTIO_DEV_GPU     0x1050

/* VirtIO PCI Register Offsets (I/O space) */
#define VIRTIO_PCI_HOST_FEATURES  0x00
#define VIRTIO_PCI_GUEST_FEATURES 0x04
#define VIRTIO_PCI_QUEUE_PFN      0x08
#define VIRTIO_PCI_QUEUE_NUM      0x0C
#define VIRTIO_PCI_QUEUE_SEL      0x0E
#define VIRTIO_PCI_QUEUE_NOTIFY   0x10
#define VIRTIO_PCI_STATUS         0x12
#define VIRTIO_PCI_ISR            0x13

/* VirtIO Device Status Bits */
#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER      2
#define VIRTIO_STATUS_DRIVER_OK   4
#define VIRTIO_STATUS_FAILED      128

typedef struct {
    uint8_t  bus;
    uint8_t  slot;
    uint8_t  func;
    uint16_t device_id;
    uint16_t vendor_id;
    uint16_t io_base;
    uint8_t  irq;
    bool     active;
} virtio_pci_dev_t;

void virtio_init(void);
int virtio_scan_pci(virtio_pci_dev_t *devs, int max_devs);
void virtio_dump_devices(void);

#endif /* DRIVERS_VIRTIO_H */
