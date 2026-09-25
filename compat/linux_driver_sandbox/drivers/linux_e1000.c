#include "../linux_env.h"
#include "../ldk_bridge.h"
#include <stdio.h>
#include <string.h>

#define E1000_VENDOR_INTEL 0x8086
#define E1000_DEV_82540EM  0x100E

static const struct pci_device_id e1000_pci_tbl[] = {
    { E1000_VENDOR_INTEL, E1000_DEV_82540EM, 0, 0, 0, 0, 0 },
    { 0, }
};

static int e1000_open(struct net_device *netdev) {
    linux_printk(KERN_INFO "e1000: Interface %s brought UP.\n", netdev->name);
    return 0;
}

static int e1000_close(struct net_device *netdev) {
    linux_printk(KERN_INFO "e1000: Interface %s brought DOWN.\n", netdev->name);
    return 0;
}

static int e1000_xmit_frame(struct sk_buff *skb, struct net_device *netdev) {
    (void)netdev;
    if (!skb) return -1;
    // Send packet frame over LDK bridge to Iodine Microkernel HAL
    return ldk_bridge_send(LDK_MSG_TX_PACKET, skb->data, skb->len);
}

static const struct net_device_ops e1000_netdev_ops = {
    .ndo_open = e1000_open,
    .ndo_stop = e1000_close,
    .ndo_start_xmit = e1000_xmit_frame
};

static int e1000_probe(struct pci_dev *pdev, const struct pci_device_id *ent) {
    (void)ent;
    linux_printk(KERN_INFO "e1000: Intel(R) PRO/1000 Gigabit Ethernet Adapter discovered.\n");
    linux_printk(KERN_INFO "e1000: Vendor 0x%04x, Device 0x%04x, IRQ %d\n", pdev->vendor, pdev->device, pdev->irq);

    struct net_device *netdev = linux_alloc_etherdev(256);
    if (!netdev) return -1;

    netdev->netdev_ops = &e1000_netdev_ops;
    pdev->dev_data = netdev;

    return linux_register_netdev(netdev);
}

static void e1000_remove(struct pci_dev *pdev) {
    if (pdev && pdev->dev_data) {
        linux_kfree(pdev->dev_data);
        linux_printk(KERN_INFO "e1000: Device removed.\n");
    }
}

static struct pci_driver e1000_driver = {
    .name = "e1000",
    .id_table = e1000_pci_tbl,
    .probe = e1000_probe,
    .remove = e1000_remove
};

int e1000_init_module(void) {
    linux_printk(KERN_INFO "e1000: Intel(R) PRO/1000 Network Driver (Linux GPL Sandbox Mode)\n");
    linux_pci_register_driver(&e1000_driver);

    // Simulate probe for Intel 82540EM
    struct pci_dev simulated_pdev;
    simulated_pdev.vendor = E1000_VENDOR_INTEL;
    simulated_pdev.device = E1000_DEV_82540EM;
    simulated_pdev.subsystem_vendor = 0x8086;
    simulated_pdev.subsystem_device = 0x001E;
    simulated_pdev.irq = 11;

    e1000_probe(&simulated_pdev, &e1000_pci_tbl[0]);
    return 0;
}
