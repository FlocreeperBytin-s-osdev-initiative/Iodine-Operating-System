#ifndef COMPAT_LDK_BRIDGE_H
#define COMPAT_LDK_BRIDGE_H

#include <types.h>

#define LDK_MSG_INIT          1
#define LDK_MSG_PROBE         2
#define LDK_MSG_TX_PACKET     3
#define LDK_MSG_RX_PACKET     4
#define LDK_MSG_BLOCK_IO      5
#define LDK_MSG_IRQ_SIGNAL    6
#define LDK_MSG_SHUTDOWN      7

#define LDK_STATUS_OK         0
#define LDK_STATUS_ERR        1
#define LDK_STATUS_BUSY       2

typedef struct {
    uint32_t type;
    uint32_t length;
    int32_t  status;
    uint32_t seq;
} ldk_msg_header_t;

typedef struct {
    ldk_msg_header_t hdr;
    uint8_t payload[1514]; // Max standard Ethernet MTU frame
} ldk_packet_msg_t;

void ldk_bridge_init(void);
int ldk_bridge_send(uint32_t type, const void *data, size_t len);
int ldk_bridge_recv(ldk_msg_header_t *out_hdr, void *out_buf, size_t max_len);

#endif /* COMPAT_LDK_BRIDGE_H */
