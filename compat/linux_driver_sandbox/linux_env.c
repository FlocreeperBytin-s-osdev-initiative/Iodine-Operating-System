#include "linux_env.h"
#include "ldk_bridge.h"
#include "../../mm/kheap.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"

void *linux_kmalloc(size_t size, int flags) {
    (void)flags;
    return kmalloc(size);
}

void linux_kfree(void *ptr) {
    kfree(ptr);
}

int linux_pci_register_driver(struct pci_driver *drv) {
    if (!drv) return -1;
    linux_printk(KERN_INFO "Registered Linux PCI Driver: %s\n", drv->name);
    return 0;
}

struct net_device *linux_alloc_etherdev(size_t sizeof_priv) {
    size_t total = sizeof(struct net_device) + sizeof_priv;
    struct net_device *dev = (struct net_device *)kmalloc(total);
    if (!dev) return NULL;
    memset(dev, 0, total);

    if (sizeof_priv > 0) {
        dev->priv = (void *)(dev + 1);
    }
    strcpy(dev->name, "eth0");
    dev->dev_addr[0] = 0x52;
    dev->dev_addr[1] = 0x54;
    dev->dev_addr[2] = 0x00;
    dev->dev_addr[3] = 0x12;
    dev->dev_addr[4] = 0x34;
    dev->dev_addr[5] = 0x78;
    return dev;
}

int linux_register_netdev(struct net_device *dev) {
    if (!dev) return -1;
    linux_printk(KERN_INFO "Registered Linux Network Device: %s (HWaddr %02x:%02x:%02x:%02x:%02x:%02x)\n",
                 dev->name,
                 dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
                 dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

    ldk_bridge_send(LDK_MSG_INIT, dev->dev_addr, 6);
    return 0;
}

int linux_netif_rx(struct sk_buff *skb) {
    if (!skb) return -1;
    return ldk_bridge_send(LDK_MSG_RX_PACKET, skb->data, skb->len);
}

void linux_printk(const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    printf("%s", buf);
}
