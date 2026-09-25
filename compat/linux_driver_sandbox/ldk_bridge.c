#include "ldk_bridge.h"
#include "../../lib/string.h"
#include "../../lib/stdio.h"

#define RING_BUFFER_SIZE 16

static ldk_packet_msg_t msg_queue[RING_BUFFER_SIZE];
static int queue_head = 0;
static int queue_tail = 0;
static uint32_t current_seq = 1;

void ldk_bridge_init(void) {
    memset(msg_queue, 0, sizeof(msg_queue));
    queue_head = 0;
    queue_tail = 0;
    current_seq = 1;
}

int ldk_bridge_send(uint32_t type, const void *data, size_t len) {
    int next = (queue_head + 1) % RING_BUFFER_SIZE;
    if (next == queue_tail) {
        return -LDK_STATUS_BUSY; // Queue full
    }

    ldk_packet_msg_t *msg = &msg_queue[queue_head];
    msg->hdr.type = type;
    msg->hdr.length = (len > sizeof(msg->payload)) ? sizeof(msg->payload) : (uint32_t)len;
    msg->hdr.status = LDK_STATUS_OK;
    msg->hdr.seq = current_seq++;

    if (data && len > 0) {
        memcpy(msg->payload, data, msg->hdr.length);
    }

    queue_head = next;
    return LDK_STATUS_OK;
}

int ldk_bridge_recv(ldk_msg_header_t *out_hdr, void *out_buf, size_t max_len) {
    if (queue_head == queue_tail) {
        return 0; // Empty
    }

    ldk_packet_msg_t *msg = &msg_queue[queue_tail];
    if (out_hdr) {
        *out_hdr = msg->hdr;
    }

    size_t copy_len = (msg->hdr.length > max_len) ? max_len : msg->hdr.length;
    if (out_buf && copy_len > 0) {
        memcpy(out_buf, msg->payload, copy_len);
    }

    queue_tail = (queue_tail + 1) % RING_BUFFER_SIZE;
    return (int)copy_len;
}
