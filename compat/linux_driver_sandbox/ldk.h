#ifndef COMPAT_LDK_H
#define COMPAT_LDK_H

#include <types.h>

#define LDK_VERSION "1.0.0"

typedef enum {
    LDK_DEV_UNKNOWN = 0,
    LDK_DEV_NET     = 1,
    LDK_DEV_BLOCK   = 2,
    LDK_DEV_CHAR    = 3
} ldk_dev_type_t;

typedef struct {
    char name[32];
    char linux_module_name[32];
    ldk_dev_type_t type;
    uint16_t vendor_id;
    uint16_t device_id;
    bool sandbox_running;
    uint32_t packets_processed;
    uint32_t bytes_transferred;
} ldk_sandbox_status_t;

void ldk_sandbox_init(void);
void ldk_sandbox_start(void);
void ldk_sandbox_stop(void);
void ldk_sandbox_dump_status(void);

#endif /* COMPAT_LDK_H */
