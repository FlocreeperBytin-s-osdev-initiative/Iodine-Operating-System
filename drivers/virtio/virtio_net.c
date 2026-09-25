#include "virtio_net.h"
#include "virtio.h"
#include "virtqueue.h"
#include "../../arch/i386/io.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"

static uint8_t device_mac[6] = { 0x52, 0x54, 0x00, 0x12, 0x34, 0x56 };
static bool net_active = false;
static uint16_t net_io_base = 0;

void virtio_net_init(void) {
    virtio_pci_dev_t devs[8];
    int count = virtio_scan_pci(devs, 8);

    for (int i = 0; i < count; i++) {
        if (devs[i].device_id == VIRTIO_DEV_NET && devs[i].io_base != 0) {
            net_io_base = devs[i].io_base;

            // Reset and configure
            outb(net_io_base + VIRTIO_PCI_STATUS, 0);
            outb(net_io_base + VIRTIO_PCI_STATUS, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER);

            // Read MAC address from device config space (offset 20 in legacy I/O space)
            for (int m = 0; m < 6; m++) {
                device_mac[m] = inb(net_io_base + 20 + m);
            }

            outb(net_io_base + VIRTIO_PCI_STATUS,
                 VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_DRIVER_OK);
            net_active = true;
            printf("[VIRTIO] VirtIO Network Interface active (MAC %02x:%02x:%02x:%02x:%02x:%02x)\n",
                   device_mac[0], device_mac[1], device_mac[2], device_mac[3], device_mac[4], device_mac[5]);
            return;
        }
    }
}

void virtio_net_get_mac(uint8_t mac[6]) {
    memcpy(mac, device_mac, 6);
}

int virtio_net_send_packet(const uint8_t *packet, size_t len) {
    (void)packet; (void)len;
    if (!net_active) return -1;
    return (int)len;
}
