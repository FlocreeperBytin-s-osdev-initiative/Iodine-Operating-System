#include "virtqueue.h"
#include "../../mm/kheap.h"
#include "../../arch/i386/io.h"
#include "../../lib/string.h"
#include "virtio.h"

virtqueue_t *virtqueue_create(uint16_t io_base, uint16_t queue_index, uint16_t queue_size) {
    if (queue_size == 0) queue_size = 16;

    virtqueue_t *vq = (virtqueue_t *)kmalloc(sizeof(virtqueue_t));
    if (!vq) return NULL;
    memset(vq, 0, sizeof(virtqueue_t));

    vq->io_base = io_base;
    vq->queue_index = queue_index;
    vq->num = queue_size;
    vq->num_free = queue_size;

    // Allocate memory for desc, avail, and used rings aligned to 4096
    size_t desc_size  = sizeof(struct vring_desc) * queue_size;
    size_t avail_size = sizeof(uint16_t) * (3 + queue_size);
    size_t used_size  = sizeof(uint16_t) * 3 + sizeof(struct vring_used_elem) * queue_size;

    size_t total_size = desc_size + avail_size + 4096 + used_size;
    vq->raw_mem = kmalloc(total_size);
    if (!vq->raw_mem) {
        kfree(vq);
        return NULL;
    }
    memset(vq->raw_mem, 0, total_size);

    uintptr_t ptr = (uintptr_t)vq->raw_mem;
    vq->desc = (struct vring_desc *)ptr;
    ptr += desc_size;
    vq->avail = (struct vring_avail *)ptr;
    ptr += avail_size;
    // Align used ring to 4096 bytes boundary as required by legacy virtio spec
    ptr = (ptr + 4095) & ~4095;
    vq->used = (struct vring_used *)ptr;

    // Initialize free list
    for (uint16_t i = 0; i < queue_size - 1; i++) {
        vq->desc[i].next = i + 1;
    }
    vq->desc[queue_size - 1].next = 0xFFFF;
    vq->free_head = 0;

    // Configure VirtIO device register
    if (io_base != 0) {
        outw(io_base + VIRTIO_PCI_QUEUE_SEL, queue_index);
        outl(io_base + VIRTIO_PCI_QUEUE_PFN, ((uint32_t)vq->raw_mem) >> 12);
    }

    return vq;
}

void virtqueue_destroy(virtqueue_t *vq) {
    if (!vq) return;
    if (vq->raw_mem) kfree(vq->raw_mem);
    kfree(vq);
}

int virtqueue_add_buf(virtqueue_t *vq, uint64_t phys_addr, uint32_t len, bool is_write) {
    if (!vq || vq->num_free == 0) return -1;

    uint16_t idx = vq->free_head;
    vq->free_head = vq->desc[idx].next;
    vq->num_free--;

    vq->desc[idx].addr = phys_addr;
    vq->desc[idx].len = len;
    vq->desc[idx].flags = is_write ? VRING_DESC_F_WRITE : 0;
    vq->desc[idx].next = 0xFFFF;

    // Add to available ring
    uint16_t avail_idx = vq->avail->idx % vq->num;
    vq->avail->ring[avail_idx] = idx;
    vq->avail->idx++;

    return idx;
}

void virtqueue_kick(virtqueue_t *vq) {
    if (vq && vq->io_base != 0) {
        outw(vq->io_base + VIRTIO_PCI_QUEUE_NOTIFY, vq->queue_index);
    }
}

bool virtqueue_has_used(virtqueue_t *vq) {
    if (!vq) return false;
    return vq->last_used_idx != vq->used->idx;
}
