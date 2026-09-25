#!/usr/bin/env python3
import sys
import os
import struct

def make_iso(img_path, iso_path):
    with open(img_path, 'rb') as f:
        img_data = f.read()

    # Sector size 2048 bytes
    SECTOR_SIZE = 2048

    # ISO structure:
    # Sectors 0-15: System Area (32768 bytes, zeroes)
    # Sector 16: Primary Volume Descriptor
    # Sector 17: Boot Record Volume Descriptor (El Torito)
    # Sector 18: Volume Descriptor Set Terminator
    # Sector 19: Boot Catalog
    # Sector 20+: Floppy Boot Image (1.44 MB)

    iso = bytearray(16 * SECTOR_SIZE)

    # Sector 16: Primary Volume Descriptor
    pvd = bytearray(SECTOR_SIZE)
    pvd[0] = 0x01 # Type: PVD
    pvd[1:6] = b'CD001' # Identifier
    pvd[6] = 0x01 # Version
    pvd[8:40] = b'IODINE_OS'.ljust(32) # System Identifier
    pvd[40:72] = b'IODINE_OS_V1'.ljust(32) # Volume Identifier
    
    # Calculate total sectors
    img_sectors = (len(img_data) + SECTOR_SIZE - 1) // SECTOR_SIZE
    total_sectors = 20 + img_sectors
    
    # Volume Space Size (both endianness)
    struct.pack_into('<I', pvd, 80, total_sectors)
    struct.pack_into('>I', pvd, 84, total_sectors)
    
    pvd[120:124] = b'\x01\x00\x00\x01' # Volume Set Size
    pvd[124:128] = b'\x01\x00\x00\x01' # Volume Sequence Number
    struct.pack_into('<H', pvd, 128, SECTOR_SIZE)
    struct.pack_into('>H', pvd, 130, SECTOR_SIZE)
    
    iso.extend(pvd)

    # Sector 17: Boot Record Volume Descriptor
    brvd = bytearray(SECTOR_SIZE)
    brvd[0] = 0x00 # Boot Record
    brvd[1:6] = b'CD001'
    brvd[6] = 0x01
    brvd[7:39] = b'EL TORITO SPECIFICATION'.ljust(32)
    # Pointer to Boot Catalog at Sector 19
    struct.pack_into('<I', brvd, 71, 19)
    iso.extend(brvd)

    # Sector 18: Volume Descriptor Set Terminator
    term = bytearray(SECTOR_SIZE)
    term[0] = 0xFF # Terminator
    term[1:6] = b'CD001'
    term[6] = 0x01
    iso.extend(term)

    # Sector 19: Boot Catalog
    cat = bytearray(SECTOR_SIZE)
    # Validation Entry
    cat[0] = 0x01 # Header ID
    cat[1] = 0x00 # Platform x86
    cat[28:30] = b'\x55\xAA' # Key 55 AA
    # Checksum calculation for bytes 0-31
    chk = sum(struct.unpack('<16H', cat[:32])) & 0xFFFF
    struct.pack_into('<H', cat, 28, (-chk) & 0xFFFF)
    struct.pack_into('<H', cat, 30, 0xAA55)

    # Initial / Default Entry (offset 32)
    cat[32] = 0x88 # Bootable
    cat[33] = 0x02 # Boot media type: 1.44MB Floppy Emulation
    cat[34:36] = b'\x00\x00' # Load Segment (0 = 0x7C0)
    cat[36] = 0x00 # System type
    cat[38:40] = b'\x01\x00' # Sector count = 1
    struct.pack_into('<I', cat, 40, 20) # Virtual disk load packet LBA = sector 20

    iso.extend(cat)

    # Sector 20+: Floppy Boot Image
    iso.extend(img_data)
    # Pad to sector boundary
    rem = len(iso) % SECTOR_SIZE
    if rem > 0:
        iso.extend(bytearray(SECTOR_SIZE - rem))

    with open(iso_path, 'wb') as f:
        f.write(iso)
    print(f"Created bootable ISO: {iso_path} ({len(iso)} bytes, {len(iso)//SECTOR_SIZE} sectors)")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: mkiso.py <image.img> <output.iso>")
        sys.exit(1)
    make_iso(sys.argv[1], sys.argv[2])
