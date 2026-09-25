# Iodine Operating System

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/platform-x86__32%20Bare--Metal-blue.svg)]()
[![Kernel](https://img.shields.io/badge/kernel-Monolithic%201.0.0-purple.svg)]()
[![No Linux/BSD](https://img.shields.io/badge/origin-100%25%20From%20Scratch-red.svg)]()
[![License](https://img.shields.io/badge/license-BSD--2--Clause-green.svg)]()

> **Iodine OS** is an independent, complete, monolithic 32-bit x86 protected mode operating system written from scratch in C and Assembly.
> **Zero lines of Linux, BSD, or foreign distro code.** Built entirely from bare metal.

```
   ___         _ _             user@iodine-pc
  |_ _|___  __| (_)_ __   ___   --------------
   | |/ _ \/ _` | | '_ \ / _ \  OS: Iodine Operating System 1.0.0
   | | (_) | (_| | | | | |  __/  Host: Bare-Metal x86 Architecture
  |___\___/ \__,_|_|_| |_|\___|  Kernel: 1.0.0-monolithic (32-bit)
                                 Uptime: 00:04:12
    [ I O D I N E   O S ]        Shell: Iodine Shell (ish)
    Pure Custom Bare Metal       Memory: 4 MB / 128 MB (3%)
    No Linux - No BSD            Date: 2026-09-25 UTC
```

---

## Highlights & Features

- **Dual Boot Architecture**:
  - **Custom MBR Bootloader (`boot/boot.s`)**: 16-bit real mode floppy/disk loader with CHS multi-sector reader, A20 gate activation, GDT configuration, 32-bit protected mode jump, and kernel memory relocation.
  - **Multiboot 1 Compliant (`boot/multiboot.s`)**: Native Multiboot header (`0x1BADB002`) allowing direct boot from GRUB, QEMU (`-kernel`), Bochs, and VirtualBox.
- **CPU & Architecture Control**:
  - Global Descriptor Table (GDT) with Kernel Code, Kernel Data, User Code, User Data, and Task State Segment (TSS).
  - Interrupt Descriptor Table (IDT) with 256 gates.
  - Full CPU Exception Handling (ISRs 0–31) with visual panic screen, CR2 fault reporting, and register dumps.
  - 8259A PIC dual-cascaded remapping to IRQs 32–47.
  - System Call Gate (`int 0x80`) with register parameter passing.
- **Hardware Drivers**:
  - **VGA Text Mode (80x25)**: Cursor control, color attributes, automatic scrolling, box/window drawing.
  - **Serial UART 16550 (COM1 0x3F8)**: Configured at 38400 baud 8-N-1. All kernel output and shell interaction mirror simultaneously to VGA and serial.
  - **Programmable Interval Timer (PIT 8254)**: 100 Hz timer tick resolution, microsecond uptime calculation.
  - **PS/2 Keyboard Controller**: Scancode set 1 decoding, Shift/Ctrl/Caps handling, arrow keys, 256-byte circular FIFO.
  - **CMOS / Real Time Clock (RTC)**: Reads real hardware calendar date and time.
  - **PC Speaker Driver**: Square wave frequency generator, acoustic beeps, and 8-bit musical themes.
  - **ATA / IDE PIO Driver**: Primary IDE master controller with 28-bit LBA sector read/write.
  - **Power Controller**: Hardware ACPI / APM poweroff and 8042 keyboard controller reset.
- **Advanced Memory Management**:
  - **Physical Memory Manager (PMM)**: Bitmap page frame allocator tracking 4KB physical pages.
  - **Virtual Memory Manager (VMM)**: Two-level x86 paging (Page Directory + Page Tables) identity-mapping 32MB physical memory.
  - **Dynamic Kernel Heap Allocator**: Boundary-tag free list allocator spanning 16MB (`0x01000000`–`0x02000000`) with adjacent block coalescing and corruption detection.
- **Virtual File System (VFS)**:
  - Unified file node abstraction (`vfs_node_t`) with path resolution.
  - **RamFS**: In-memory hierarchical directory tree with dynamically expandable file buffers.
  - **DevFS (`/dev`)**: `/dev/null`, `/dev/zero`, `/dev/random`, `/dev/serial`, `/dev/vga`, `/dev/keyboard`.
  - **ProcFS (`/proc`)**: `/proc/version`, `/proc/meminfo`, `/proc/uptime`, `/proc/cpuinfo`, `/proc/tasks`.
- **Preemptive Multitasking Scheduler**:
  - Process Control Block (PCB) tracking PID, state, stack, priority, and CPU runtime.
  - Preemptive round-robin scheduler triggered on timer ticks.
  - Low-level assembly context switcher (`switch_task`).
- **Interactive Shell (`ish`) & Applications**:
  - Prompt: `iodine:[path]# ` with syntax highlighting.
  - History buffer (Up/Down arrow keys recall previous commands).
  - Tab auto-completion for command names and file paths.
  - Line editing (Left/Right arrow, Backspace, Ctrl+L, Ctrl+C).
  - Over 30 built-in commands (`help`, `ls -l`, `cat`, `touch`, `mkdir`, `rm`, `write`, `mem`, `ps`, `top`, `calc`, `date`, `uptime`, `color`, `hexdump`, `wc`, etc.).
  - **Snake Arcade Game**: Playable retro snake game on VGA text console with sound effects and scoring.
  - **Visual Text Editor ("Iodine Edit")**: Full-screen Nano-style text editor with save/load capability.
  - **Matrix Rain**: Falling digital green character rain screen saver.
  - **Arithmetic Calculator**: Math expression solver with operator precedence.
- **Live Web Preview & Emulator**:
  - Runs in the browser via WebAssembly x86 PC emulation with real VGA canvas and serial console.

---

## Codebase Organization

```
Iodine-Operating-System/
├── Makefile                 # Turnkey build system
├── README.md                # Project documentation
├── build.sh                 # One-step build script
├── test.sh                  # Automated test verification suite
├── arch/i386/               # CPU & Low-level architecture
│   ├── gdt.c / gdt.h        # Global Descriptor Table & TSS
│   ├── gdt_flush.s          # GDT reload & segment flush
│   ├── idt.c / idt.h        # Interrupt Descriptor Table (256 gates)
│   ├── interrupts.s         # ISR and IRQ assembly stubs
│   ├── isr.c / isr.h        # CPU exception & IRQ dispatcher
│   ├── pic.c / pic.h        # 8259A PIC controller
│   └── io.h                 # Port I/O instructions (inb, outb, etc.)
├── boot/                    # Bootstrapping
│   ├── boot.s               # Custom 16-bit MBR bootloader
│   ├── multiboot.s          # 32-bit Multiboot entry point
│   ├── multiboot.h          # Multiboot specification headers
│   └── linker.ld            # Kernel linker script (1MB base)
├── drivers/                 # Hardware device drivers
│   ├── vga.c / vga.h        # VGA 80x25 text mode display
│   ├── serial.c / serial.h  # 16550 UART COM1 serial controller
│   ├── timer.c / timer.h    # PIT 8254 timer (100 Hz)
│   ├── keyboard.c / .h      # PS/2 keyboard controller
│   ├── rtc.c / rtc.h        # Real-time clock (CMOS)
│   ├── speaker.c / .h       # PC speaker sound driver
│   ├── ata.c / ata.h        # ATA / IDE PIO controller
│   └── power.c / power.h    # ACPI poweroff & keyboard reset
├── mm/                      # Memory management
│   ├── pmm.c / pmm.h        # Physical memory manager (bitmap)
│   ├── vmm.c / vmm.h        # Virtual memory manager (paging)
│   └── kheap.c / kheap.h    # Dynamic kernel heap allocator
├── fs/                      # File systems
│   ├── vfs.c / vfs.h        # Virtual File System abstraction
│   ├── ramfs.c / ramfs.h    # In-memory RamFS file system
│   ├── devfs.c / devfs.h    # /dev devices (null, zero, serial, etc.)
│   └── procfs.c / procfs.h  # /proc pseudo-filesystem
├── kernel/                  # Kernel core
│   ├── main.c               # Kernel initialization entry point
│   ├── process.c / .h       # Process control & management
│   ├── sched.c / sched.h    # Preemptive round-robin scheduler
│   ├── task_switch.s        # Assembly context switch
│   ├── syscall.c / .h       # System call interface (int 0x80)
│   └── panic.c / panic.h    # Kernel panic handler
├── lib/                     # Standard library (libk)
│   ├── string.c / string.h  # String and memory manipulation
│   ├── stdio.c / stdio.h    # Formatted printing (printf, sprintf)
│   ├── stdlib.c / stdlib.h  # Utilities, atoi, itoa, rand
│   └── ctype.c / ctype.h    # Character classification
├── shell/                   # Interactive command line
│   ├── ish.c / ish.h        # Iodine Shell loop & line editor
│   └── commands.c / .h      # Built-in shell commands
├── apps/                    # User applications
│   ├── snake.c / snake.h    # VGA text Snake arcade game
│   ├── editor.c / editor.h  # Full-screen visual text editor
│   ├── matrix.c / matrix.h  # Digital green Matrix rain
│   └── calc.c / calc.h      # Arithmetic calculator
├── tools/
│   └── mkiso.py             # El Torito bootable ISO builder
├── web/                     # Live Web preview emulator
│   ├── server.js            # Node HTTP server on port 3000
│   ├── run.js               # CLI interactive emulator
│   └── public/              # WebAssembly emulator assets & UI
└── docs/                    # Technical manuals
    ├── ARCHITECTURE.md      # Detailed kernel architecture manual
    └── COMMANDS.md          # Shell command reference guide
```

---

## Building the OS

To build all bootable targets:

```bash
make clean
make all
```

This generates:
- `bin/iodine.img`: Bootable 1.44MB Floppy Image (contains custom MBR bootloader + raw kernel)
- `bin/iodine.iso`: Bootable El Torito CD-ROM ISO Image
- `bin/iodine.elf`: Multiboot 1 compliant ELF kernel executable

---

## Running Iodine OS

### 1. In Live Web Preview
Start the web preview server:
```bash
node web/server.js
```
Open your browser at `http://localhost:3000` to interact with the full VGA canvas display, keyboard capture, and command toolbar!

### 2. In Terminal CLI
Run directly in the console using the headless x86 engine:
```bash
make run
```

### 3. In QEMU
Boot from the floppy image:
```bash
qemu-system-i386 -fda bin/iodine.img
```
Or boot from the ISO:
```bash
qemu-system-i386 -cdrom bin/iodine.iso
```
Or boot directly via Multiboot:
```bash
qemu-system-i386 -kernel bin/iodine.elf
```

### 4. On Real Hardware
Flash `bin/iodine.img` to a USB drive using Rufus or `dd`:
```bash
dd if=bin/iodine.img of=/dev/sdX bs=4M status=progress
```

---

## Automated Verification Tests

Run the built-in automated test suite:
```bash
./test.sh
```
This boots Iodine OS in a virtual machine, sends commands via serial console, and validates kernel output, memory allocation, filesystem operations, and process scheduling.
