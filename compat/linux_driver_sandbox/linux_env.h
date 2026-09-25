#ifndef COMPAT_LINUX_ENV_H
#define COMPAT_LINUX_ENV_H

#include <types.h>

#define KERN_INFO    "[Linux Driver Sandbox] "
#define KERN_ERR     "[Linux Driver Sandbox ERROR] "
#define GFP_KERNEL   0

typedef uint32_t spinlock_t;
#define spin_lock_init(lock) *(lock) = 0
#define spin_lock(lock)       *(lock) = 1
#define spin_unlock(lock)     *(lock) = 0

struct pci_device_id {
    uint32_t vendor;
    uint32_t device;
    uint32_t subvendor;
    uint32_t subdevice;
    uint32_t class_mask;
    uint32_t class_;
    uintptr_t driver_data;
};

struct pci_dev {
    uint16_t vendor;
    uint16_t device;
    uint16_t subsystem_vendor;
    uint16_t subsystem_device;
    uint8_t  irq;
    void     *dev_data;
};

struct pci_driver {
    const char *name;
    const struct pci_device_id *id_table;
    int  (*probe)(struct pci_dev *dev, const struct pci_device_id *id);
    void (*remove)(struct pci_dev *dev);
};

struct sk_buff {
    uint8_t  *data;
    uint32_t len;
    void     *dev;
};

struct net_device;

struct net_device_ops {
    int  (*ndo_open)(struct net_device *dev);
    int  (*ndo_stop)(struct net_device *dev);
    int  (*ndo_start_xmit)(struct sk_buff *skb, struct net_device *dev);
};

struct net_device {
    char name[16];
    uint8_t dev_addr[6];
    const struct net_device_ops *netdev_ops;
    void *priv;
};

/* Linux Driver API functions */
void *linux_kmalloc(size_t size, int flags);
void  linux_kfree(void *ptr);
int   linux_pci_register_driver(struct pci_driver *drv);
struct net_device *linux_alloc_etherdev(size_t sizeof_priv);
int   linux_register_netdev(struct net_device *dev);
int   linux_netif_rx(struct sk_buff *skb);
void  linux_printk(const char *fmt, ...);

#endif /* COMPAT_LINUX_ENV_H */
