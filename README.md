# Iodine Operating System

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/platform-x86__64%20%2F%20x86__32-blue.svg)]()
[![POSIX](https://img.shields.io/badge/POSIX-Compliant-orange.svg)]()
[![musl](https://img.shields.io/badge/musl-ABI%20Compatible-teal.svg)]()
[![VirtIO](https://img.shields.io/badge/VirtIO-Net%20%26%20Block-yellow.svg)]()
[![Package Manager](https://img.shields.io/badge/package%20mgr-Alpine%20apk%20clone-purple.svg)]()
[![GPL Barrier](https://img.shields.io/badge/Linux%20Driver-Sandboxed%20(GPL%20Clean)-red.svg)]()
[![License](https://img.shields.io/badge/license-BSD--2--Clause-green.svg)]()

> **Iodine OS** is an independent, monolithic 32-bit and 64-bit x86 operating system written from scratch in C and Assembly.
> **Zero lines of Linux, BSD, or foreign distro code in the kernel.** Built entirely from bare metal.

```
   ___         _ _             user@iodine-pc
  |_ _|___  __| (_)_ __   ___   --------------
   | |/ _ \/ _` | | '_ \ / _ \  OS: Iodine Operating System 1.0.0
   | | (_) | (_| | | | | |  __/  Host: Bare-Metal x86 Architecture
  |___\___/ \__,_|_|_| |_|\___|  Kernel: 1.0.0-monolithic (32/64-bit)
                                 Uptime: 00:04:12
    [ I O D I N E   O S ]        Shell: Iodine Shell (ish)
    Pure Custom Bare Metal       Memory: 4 MB / 128 MB (3%)
    No Linux - No BSD            Date: 2026-09-25 UTC
```

---

## What Makes Iodine OS Unique?

1. **64-Bit Architecture (`arch/x86_64/`)**:
   - 64-bit long mode with 4-level paging (PML4, PDPT, PD, PT) and canonical 64-bit address space.
   - AMD64/Intel 64 fast system calls (`syscall` / `sysret`) via MSR configuration (`EFER`, `STAR`, `LSTAR`, `SFMASK`).
   - Native 64-bit ELF kernel build target (`bin/iodine64.elf`).
2. **POSIX Compliance & Subsystem (`compat/posix/`)**:
   - Full file descriptor table (0..63) with standard streams (stdin, stdout, stderr).
   - POSIX APIs: `open`, `read`, `write`, `close`, `lseek`, `stat`, `fstat`, `dup`, `dup2`, `mkdir`, `unlink`, `mmap`, `munmap`, `brk`.
   - Complete POSIX errno implementation.
3. **Linux Syscall Translator (`compat/linux/`)**:
   - An in-kernel translation layer (like Windows WSL1 or FreeBSD's Linuxulator) that translates standard Linux system calls (e.g. `sys_read`, `sys_write`, `sys_open`, `sys_stat`, `sys_uname`, `sys_clock_gettime`, `sys_getpid`, `sys_brk`) to Iodine's native POSIX/VFS operations.
   - Translates binary structures between Linux (`linux_stat64`, `linux_utsname`, `linux_timespec`, `linux_iovec`) and Iodine.
4. **musl libc Compatibility Layer (`compat/musl/`)**:
   - ABI-compliant headers and runtime wrappers for musl libc, enabling musl-targeted software to run directly on Iodine OS.
5. **Trusty ol' VirtIO Subsystem (`drivers/virtio/`)**:
   - Automatic PCI configuration space probing for VirtIO devices (`Vendor ID 0x1AF4`).
   - Split Virtqueue implementation (Descriptor table, Available ring, Used ring) with 4096-byte alignment.
   - VirtIO Block Device driver (`virtio-blk`) and Network Adapter (`virtio-net`).
6. **Linux Driver Sandbox (LDK - GPL Contamination Barrier) (`compat/linux_driver_sandbox/`)**:
   - **The Problem**: Running Linux drivers directly inside a BSD-licensed kernel legally triggers GPLv2 contamination.
   - **The Architectural Solution**: The Linux driver executes inside an isolated microkernel/user-mode sandbox domain.
   - The sandbox exposes a synthetic Linux internal driver API (`pci_register_driver()`, `alloc_etherdev()`, `register_netdev()`, `kmalloc()`, `printk()`).
   - Communication between the Iodine kernel and the sandbox crosses an arms-length, clean-room IPC bridge (`ldk_bridge`).
   - Includes a concrete driver (`drivers/linux_e1000.c` for Intel PRO/1000 Gigabit Ethernet) running in the sandbox, passing network frames without contaminating the Iodine kernel with GPL code!
7. **Alpine's `apk` Clone (`apps/apk/` / `ipk`)**:
   - Command line package manager matching Alpine Linux `apk` syntax.
   - Commands: `apk add`, `apk del`, `apk list`, `apk info`, `apk search`, `apk update`, `apk upgrade`.
   - Preloaded repository with `musl`, `bsdutils`, `virtio-tools`, `ldk-sandbox`, `curl`, `lua`, `busybox-posix`.
8. **BSD Utilities Suite (`bsdutils`) (`apps/bsdutils/`)**:
   - Suite of classic BSD tools: `cal` (calendar), `hexdump -C` (canonical hex dump), `column` (column formatting), `banner` (ASCII billboard banner), `morse` (Morse encoder), `cksum` (POSIX 32-bit CRC), `whoami`.
9. **Interactive Shell & Applications**:
   - Iodine Shell (`ish`) with history, tab completion, line editing, and over 45 commands.
   - Arcade Snake game, full-screen Nano-style visual editor, Matrix digital rain, math calculator, PC speaker melodies.
10. **Live Web Preview**:
    - Complete WebAssembly x86 emulator running in your browser on port 3000 with real-time VGA canvas, serial mirror, and download buttons for disk images!

---

## Codebase Layout

```
Iodine-Operating-System/
├── Makefile                 # Turnkey build system (32-bit & 64-bit targets)
├── README.md                # Project manual
├── build.sh                 # Single-command build
├── test.sh                  # Automated test verification suite
├── arch/
│   ├── i386/                # 32-bit CPU, GDT, IDT, PIC, ISRs
│   └── x86_64/              # 64-bit Long Mode, 4-level MMU, fast SYSCALL
├── boot/                    # 16-bit MBR bootloader & 32-bit Multiboot
├── compat/
│   ├── posix/               # POSIX file descriptors, stat, fcntl, brk, mmap
│   ├── linux/               # Linux Syscall Translator (WSL/Linuxulator style)
│   ├── musl/                # musl libc ABI compatibility
│   └── linux_driver_sandbox/# LDK Sandbox (GPL Contamination Barrier)
│       └── drivers/         # Sandboxed Linux drivers (e1000 Gigabit)
├── drivers/
│   ├── vga.c / serial.c     # Video & Serial UART COM1
│   ├── keyboard.c / timer.c # PS/2 keyboard & PIT 8254 timer
│   ├── rtc.c / speaker.c    # CMOS clock & PC speaker sound
│   ├── ata.c / power.c      # ATA hard disk & ACPI shutdown
│   └── virtio/              # Trusty ol' VirtIO (PCI, Virtqueue, Block, Net)
├── mm/                      # PMM (bitmap), VMM (paging), Kernel Heap (16MB)
├── fs/                      # VFS with RamFS, DevFS (/dev), ProcFS (/proc)
├── kernel/                  # Process PCB, preemptive scheduler, syscalls
├── lib/                     # libc / libk (string, stdio, stdlib, ctype)
├── shell/                   # Interactive shell `ish` & commands
├── apps/
│   ├── apk/                 # Alpine apk clone package manager
│   ├── bsdutils/            # BSD tools (cal, hexdump, banner, morse, cksum)
│   └── snake / editor / matrix / calc
├── tools/                   # El Torito ISO generator
└── web/                     # Live Web preview emulator on port 3000
```

---

## Building & Testing

```bash
# Build all targets (Floppy IMG, CD-ROM ISO, Multiboot ELF, 64-bit Kernel ELF)
./build.sh

# Run automated verification test suite
./test.sh
```

### Generated Artifacts
- `bin/iodine.img`: 1.44MB bootable Floppy disk image
- `bin/iodine.iso`: El Torito bootable CD-ROM image
- `bin/iodine.elf`: 32-bit Multiboot 1 compliant ELF kernel
- `bin/iodine64.elf`: 64-bit x86_64 ELF kernel

---

## Running Iodine OS

### 1. In Browser Live Preview
```bash
node web/server.js
```
Open `http://localhost:3000` to interact with the OS in real-time!

### 2. In Terminal CLI
```bash
make run
```

### 3. In QEMU
```bash
# Floppy
qemu-system-i386 -fda bin/iodine.img

# CD-ROM ISO
qemu-system-i386 -cdrom bin/iodine.iso

# Direct Multiboot
qemu-system-i386 -kernel bin/iodine.elf

# With VirtIO Block & Net:
qemu-system-i386 -fda bin/iodine.img -device virtio-net-pci -drive file=bin/iodine.img,if=virtio
```
