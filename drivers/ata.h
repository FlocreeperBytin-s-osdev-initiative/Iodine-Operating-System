#ifndef DRIVERS_ATA_H
#define DRIVERS_ATA_H

#include <types.h>

#define ATA_SECTOR_SIZE 512

void ata_init(void);
int ata_read_sectors(uint32_t lba, uint8_t count, void *buffer);
int ata_write_sectors(uint32_t lba, uint8_t count, const void *buffer);

#endif /* DRIVERS_ATA_H */
