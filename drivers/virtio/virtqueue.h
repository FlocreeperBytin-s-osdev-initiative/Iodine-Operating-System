#ifndef DRIVERS_VIRTIO_VIRTQUEUE_H
#define DRIVERS_VIRTIO_VIRTQUEUE_H

#include <types.h>

#define VRING_DESC_F_NEXT     1
#define VRING_DESC_F_WRITE    2
#define VRING_DESC_F_INDIRECT 4

struct vring_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

struct vring_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];
} __attribute__((packed));

struct vring_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

struct vring_used {
    uint16_t flags;
    uint16_t idx;
    struct vring_used_elem ring[];
} __attribute__((packed));

typedef struct {
    uint16_t num;
    struct vring_desc *desc;
    struct vring_avail *avail;
    struct vring_used *used;
    uint16_t last_used_idx;
    uint16_t free_head;
    uint16_t num_free;
    void *raw_mem;
    uint16_t io_base;
    uint16_t queue_index;
} virtqueue_t;

virtqueue_t *virtqueue_create(uint16_t io_base, uint16_t queue_index, uint16_t queue_size);
void virtqueue_destroy(virtqueue_t *vq);
int virtqueue_add_buf(virtqueue_t *vq, uint64_t phys_addr, uint32_t len, bool is_write);
void virtqueue_kick(virtqueue_t *vq);
bool virtqueue_has_used(virtqueue_t *vq);

#endif /* DRIVERS_VIRTIO_VIRTQUEUE_H */
