#ifndef DRIVERS_VIRTIO_NET_H
#define DRIVERS_VIRTIO_NET_H

#include <types.h>

struct virtio_net_hdr {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
} __attribute__((packed));

void virtio_net_init(void);
void virtio_net_get_mac(uint8_t mac[6]);
int virtio_net_send_packet(const uint8_t *packet, size_t len);

#endif /* DRIVERS_VIRTIO_NET_H */
