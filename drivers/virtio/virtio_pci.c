#include "virtio.h"
#include "../../arch/i386/io.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"

#define MAX_VIRTIO_DEVS 8
static virtio_pci_dev_t virtio_devices[MAX_VIRTIO_DEVS];
static int virtio_device_count = 0;

static uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((1 << 31) |
                                  ((uint32_t)bus << 16) |
                                  ((uint32_t)slot << 11) |
                                  ((uint32_t)func << 8) |
                                  (offset & 0xFC));
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t val = pci_read_config(bus, slot, func, offset);
    return (uint16_t)((val >> ((offset & 2) * 8)) & 0xFFFF);
}

int virtio_scan_pci(virtio_pci_dev_t *devs, int max_devs) {
    virtio_device_count = 0;

    for (uint16_t bus = 0; bus < 8; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint16_t vendor = pci_read_config_word(bus, slot, 0, 0x00);
            if (vendor == 0xFFFF) continue;

            uint16_t device = pci_read_config_word(bus, slot, 0, 0x02);
            if (vendor == VIRTIO_VENDOR_ID && virtio_device_count < MAX_VIRTIO_DEVS && virtio_device_count < max_devs) {
                virtio_pci_dev_t *d = &virtio_devices[virtio_device_count];
                d->bus = bus;
                d->slot = slot;
                d->func = 0;
                d->vendor_id = vendor;
                d->device_id = device;

                // Read BAR0 (I/O base)
                uint32_t bar0 = pci_read_config(bus, slot, 0, 0x10);
                if (bar0 & 1) { // I/O Space
                    d->io_base = (uint16_t)(bar0 & ~0x3);
                } else {
                    d->io_base = 0;
                }

                // Read Interrupt Line
                d->irq = (uint8_t)(pci_read_config(bus, slot, 0, 0x3C) & 0xFF);
                d->active = true;

                if (devs) {
                    devs[virtio_device_count] = *d;
                }
                virtio_device_count++;
            }
        }
    }
    return virtio_device_count;
}

void virtio_dump_devices(void) {
    printf("===============================================================\n");
    printf("                  VIRTIO PCI HARDWARE BUS                      \n");
    printf("===============================================================\n");
    if (virtio_device_count == 0) {
        printf("  No physical VirtIO PCI hardware found (running in pure IDE/VGA mode).\n");
        printf("  VirtIO subsystem initialized and ready for QEMU/KVM virtio devices.\n");
    } else {
        for (int i = 0; i < virtio_device_count; i++) {
            const char *type_name = "Unknown VirtIO";
            switch (virtio_devices[i].device_id) {
                case VIRTIO_DEV_NET:     type_name = "VirtIO Network Adapter"; break;
                case VIRTIO_DEV_BLOCK:   type_name = "VirtIO Block Device"; break;
                case VIRTIO_DEV_CONSOLE: type_name = "VirtIO Console"; break;
                case VIRTIO_DEV_RNG:     type_name = "VirtIO Hardware RNG"; break;
                case VIRTIO_DEV_BALLOON: type_name = "VirtIO Memory Balloon"; break;
                case VIRTIO_DEV_GPU:     type_name = "VirtIO GPU Accelerator"; break;
            }
            printf("  [#%d] Bus %02x:%02x.%d | Dev 0x%04x | I/O 0x%04x | IRQ %2d | %s\n",
                   i, virtio_devices[i].bus, virtio_devices[i].slot, virtio_devices[i].func,
                   virtio_devices[i].device_id, virtio_devices[i].io_base, virtio_devices[i].irq, type_name);
        }
    }
    printf("===============================================================\n");
}

void virtio_init(void) {
    virtio_scan_pci(virtio_devices, MAX_VIRTIO_DEVS);
}
