#ifndef DRIVERS_VIRTIO_BLK_H
#define DRIVERS_VIRTIO_BLK_H

#include <types.h>

#define VIRTIO_BLK_T_IN           0
#define VIRTIO_BLK_T_OUT          1
#define VIRTIO_BLK_T_FLUSH        4

#define VIRTIO_BLK_S_OK           0
#define VIRTIO_BLK_S_IOERR        1
#define VIRTIO_BLK_S_UNSUPP       2

struct virtio_blk_req {
    uint32_t type;
    uint32_t reserved;
    uint64_t sector;
} __attribute__((packed));

void virtio_blk_init(void);
int virtio_blk_read(uint64_t sector, uint8_t *buffer, size_t count);
int virtio_blk_write(uint64_t sector, const uint8_t *buffer, size_t count);

#endif /* DRIVERS_VIRTIO_BLK_H */
