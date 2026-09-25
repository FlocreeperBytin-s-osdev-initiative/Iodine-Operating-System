#include "ata.h"
#include "../arch/i386/io.h"

#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERR          0x1F1
#define ATA_PRIMARY_SECCOUNT     0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_HEAD   0x1F6
#define ATA_PRIMARY_COMM_STATUS  0x1F7

#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_ERR  0x01

static void ata_wait_bsy(void) {
    while (inb(ATA_PRIMARY_COMM_STATUS) & ATA_STATUS_BSY);
}

static void ata_wait_drq(void) {
    while (!(inb(ATA_PRIMARY_COMM_STATUS) & ATA_STATUS_DRQ));
}

void ata_init(void) {
    // Select master drive
    outb(ATA_PRIMARY_DRIVE_HEAD, 0xA0);
}

int ata_read_sectors(uint32_t lba, uint8_t count, void *buffer) {
    uint16_t *buf = (uint16_t *)buffer;

    outb(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, count);
    outb(ATA_PRIMARY_LBA_LO, (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMM_STATUS, 0x20); // 0x20: Read Sectors

    for (int s = 0; s < count; s++) {
        ata_wait_bsy();
        ata_wait_drq();

        for (int i = 0; i < 256; i++) {
            *buf++ = inw(ATA_PRIMARY_DATA);
        }
    }

    return 0;
}

int ata_write_sectors(uint32_t lba, uint8_t count, const void *buffer) {
    const uint16_t *buf = (const uint16_t *)buffer;

    outb(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, count);
    outb(ATA_PRIMARY_LBA_LO, (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMM_STATUS, 0x30); // 0x30: Write Sectors

    for (int s = 0; s < count; s++) {
        ata_wait_bsy();
        ata_wait_drq();

        for (int i = 0; i < 256; i++) {
            outw(ATA_PRIMARY_DATA, *buf++);
        }
    }

    // Flush cache
    outb(ATA_PRIMARY_COMM_STATUS, 0xE7);
    ata_wait_bsy();

    return 0;
}
