# Iodine OS - Kernel & System Architecture

**Iodine OS** is an independent, monolithic, 32-bit x86 protected mode operating system written from scratch in C and x86 Assembly. It contains **no code or dependencies from Linux, BSD, Unix, or any existing distribution**.

---

## 1. System Memory Map

```
+------------------------------------+ 0xFFFFFFFF (4 GB)
|           High Memory              |
+------------------------------------+ 0x02000000 (32 MB)
|        Kernel Heap (16 MB)         | (kmalloc / kfree dynamic allocations)
+------------------------------------+ 0x01000000 (16 MB)
|      Identity Mapped Memory        | (Page directory & page tables)
+------------------------------------+ 0x00400000 (4 MB)
|      Kernel Code & Data (.text)    | (ELF loaded at 1MB mark)
+------------------------------------+ 0x00100000 (1 MB)
| BIOS / Extended System Data Area   |
+------------------------------------+ 0x000A0000 (640 KB)
| VGA Color Text Buffer (0xB8000)   | (80x25 character matrix)
+------------------------------------+ 0x000B8000
| Low Conventional Memory (Stacks)  |
+------------------------------------+ 0x00010000 (64 KB)
| MBR Bootloader (Loaded by BIOS)    |
+------------------------------------+ 0x00007C00 (31 KB)
| BIOS Interrupt Vector Table (IVT)  |
+------------------------------------+ 0x00000000 (0 KB)
```

---

## 2. Bootstrapping Sequence

Iodine OS supports two distinct boot mechanisms:

### Mode A: Custom Two-Stage MBR Bootloader (`boot/boot.s`)
1. **Real Mode (16-bit)**: BIOS loads sector 0 of the boot media into `0x0000:0x7C00`.
2. **Drive Detection & Sector Loading**: Bootloader queries drive parameters via BIOS INT 13h and executes a multi-sector LBA-to-CHS read loop loading 240 sectors (120 KB) of the kernel binary into `0x1000:0x0000` (`0x10000`).
3. **A20 Gate Activation**: Enables the Fast A20 gate via System Control Port A (`0x92`) to unlock physical memory above 1MB.
4. **GDT Setup**: Loads a flat 4GB descriptor table with 32-bit Ring 0 Code and Data segments.
5. **Protected Mode Switch**: Sets bit 0 (PE) of Control Register `CR0`, clearing interrupts (`cli`), and performs a far jump `jmp 0x08:pm_start` to flush the CPU pipeline.
6. **Kernel Relocation**: In 32-bit protected mode, copies the kernel dwords from `0x10000` to `0x100000` (1MB mark).
7. **Execution**: Passes simulated Multiboot magic `0x2BADB002` in `EAX` and jumps to `0x100000`.

### Mode B: Multiboot 1 Specification (`boot/multiboot.s`)
1. Complies with the Free Software Foundation Multiboot specification (`0x1BADB002`).
2. Loaded directly at `0x100000` by bootloaders such as GRUB, QEMU `-kernel`, or SeaBIOS.
3. Sets up an initial 32KB kernel stack in the `.bss` section and passes multiboot memory information pointers to `kernel_main`.

---

## 3. CPU & Interrupt Architecture

### Global Descriptor Table (GDT)
Located in `arch/i386/gdt.c`:
- **Index 0 (0x00)**: Null Descriptor
- **Index 1 (0x08)**: Kernel Code Segment (Base: `0x0`, Limit: `4GB`, DPL: 0, 32-bit Read/Execute)
- **Index 2 (0x10)**: Kernel Data Segment (Base: `0x0`, Limit: `4GB`, DPL: 0, 32-bit Read/Write)
- **Index 3 (0x18 | 3 = 0x1B)**: User Code Segment (Base: `0x0`, Limit: `4GB`, DPL: 3, 32-bit Read/Execute)
- **Index 4 (0x20 | 3 = 0x23)**: User Data Segment (Base: `0x0`, Limit: `4GB`, DPL: 3, 32-bit Read/Write)
- **Index 5 (0x28)**: Task State Segment (TSS) holding `esp0` for user-to-kernel stack transitions.

### Interrupt Descriptor Table (IDT) & PIC Remapping
Located in `arch/i386/idt.c`, `arch/i386/pic.c`, `arch/i386/interrupts.s`:
- **256 Interrupt Gates**: Gate attributes `0x8E` (Ring 0 Interrupt Gate) and `0xEE` (Ring 3 System Call Gate).
- **Exceptions 0–31**: Traps for Division by Zero, Invalid Opcode, Double Fault, General Protection Fault (`#GP`), and Page Fault (`#PF`). Handled by `arch/i386/isr.c` with diagnostic register dumps.
- **8259A PIC Remapping**: Remapped from BIOS default IRQs to vectors 32–47:
  - IRQ 0 (Vector 32): Programmable Interval Timer (PIT)
  - IRQ 1 (Vector 33): PS/2 Keyboard Controller
  - IRQ 4 (Vector 36): COM1 Serial UART
  - IRQ 8 (Vector 40): CMOS Real Time Clock
  - IRQ 14 (Vector 46): Primary ATA/IDE Disk Controller
- **System Call Gate (Vector 128 / 0x80)**: Ring 3 accessible software interrupt gate for userspace applications.

---

## 4. Hardware Drivers

1. **VGA Text Mode Driver (`drivers/vga.c`)**:
   - Memory buffer at `0xB8000`.
   - Cursor manipulation via CRT Controller registers (`0x3D4` and `0x3D5`).
   - 16 colors, auto-scrolling, backspace handling, window/box drawing.
2. **Serial UART 16550 (`drivers/serial.c`)**:
   - Port `0x3F8` (COM1).
   - Configured for 38400 baud, 8-N-1, FIFO enabled.
   - Dual-mirror console: all kernel output and shell text echo to both VGA display and serial console.
3. **Programmable Interval Timer (`drivers/timer.c`)**:
   - Channel 0 configured in Mode 3 (Square Wave Generator).
   - Frequency: 100 Hz (10 ms resolution).
   - Tracks global system ticks and uptime.
4. **PS/2 Keyboard Driver (`drivers/keyboard.c`)**:
   - Port `0x60` (data) and `0x64` (status/control).
   - Scancode set 1 decoder handling shift, caps lock, control keys, arrow keys.
   - 256-byte circular FIFO ring buffer.
5. **Real Time Clock / CMOS (`drivers/rtc.c`)**:
   - CMOS Ports `0x70` / `0x71`.
   - BCD-to-binary decoding, 24-hour conversion, century calculation.
6. **PC Speaker Driver (`drivers/speaker.c`)**:
   - Channel 2 PIT counter at `0x42` with speaker gate at `0x61`.
   - Generates square-wave audio frequencies, beeps, and musical melodies.
7. **ATA PIO Storage Controller (`drivers/ata.c`)**:
   - Primary Master IDE controller on ports `0x1F0`–`0x1F7`.
   - 28-bit LBA PIO sector read/write operations.
8. **Power Management (`drivers/power.c`)**:
   - APM/ACPI power off via ports `0x604`, `0xB004`, `0x4004`.
   - Soft reboot pulse via 8042 keyboard controller (`0x64` -> `0xFE`).

---

## 5. Memory Management

1. **Physical Memory Manager (`mm/pmm.c`)**:
   - Page Frame Allocator using a bitmap tracking physical 4096-byte pages.
   - First 4MB reserved for kernel, page tables, and hardware buffers.
   - Functions: `pmm_alloc_block()`, `pmm_free_block()`, memory counters.
2. **Virtual Memory Manager (`mm/vmm.c`)**:
   - Standard x86 Two-Level Paging: Page Directory (1024 PDEs) and Page Tables (1024 PTEs).
   - Identity-maps the lower 32 MB of physical memory.
   - Page fault interrupt handler (Vector 14) reads `CR2` and error flags.
3. **Kernel Heap Allocator (`mm/kheap.c`)**:
   - Boundary-tag free list allocator from `0x01000000` to `0x02000000` (16 MB).
   - Magic guard word `0xCAFEBABE` detects memory corruption.
   - Adjacent block coalescing on `kfree`.
   - `kmalloc`, `kfree`, `kcalloc`, `krealloc`.

---

## 6. Virtual File System (VFS)

Located in `fs/vfs.c`:
- Uniform abstraction using `vfs_node_t` supporting `read`, `write`, `open`, `close`, `readdir`, `finddir`, `mkdir`, `unlink`.
- **RamFS (`fs/ramfs.c`)**:
  - In-memory hierarchical directory tree.
  - Dynamically resizable file buffers.
  - Preloaded with `/README.txt`, `/etc/`, `/home/user/`, `/bin/`, `/docs/`.
- **DevFS (`fs/devfs.c`)**:
  - Mounted at `/dev`.
  - `/dev/null`, `/dev/zero`, `/dev/random`, `/dev/serial`, `/dev/vga`, `/dev/keyboard`.
- **ProcFS (`fs/procfs.c`)**:
  - Mounted at `/proc`.
  - Dynamic pseudo-files: `/proc/version`, `/proc/meminfo`, `/proc/uptime`, `/proc/cpuinfo`, `/proc/tasks`.

---

## 7. Multitasking & Scheduler

Located in `kernel/process.c`, `kernel/sched.c`, `kernel/task_switch.s`:
- **Process Control Block (PCB)**:
  - PID, process name, execution state (`READY`, `RUNNING`, `SLEEPING`, `BLOCKED`, `ZOMBIE`).
  - Saved stack pointer `esp`, `ebp`, allocated 16KB stack, priority, CPU tick counter.
- **Round-Robin Preemptive Scheduler**:
  - Triggered on PIT timer interrupts (IRQ 0).
  - Wakes up sleeping tasks whose sleep timer has expired.
  - Context switch via assembly routine `switch_task(prev_esp, next_esp)` that switches CPU stacks and restores callee-saved registers.

---

## 8. System Calls (INT 0x80)

| Syscall ID | Name | Description |
|:---|:---|:---|
| 0 | `SYS_EXIT` | Terminate process |
| 1 | `SYS_FORK` | Fork task |
| 2 | `SYS_READ` | Read bytes from file descriptor |
| 3 | `SYS_WRITE` | Write bytes to file descriptor |
| 7 | `SYS_GETPID` | Get current process ID |
| 8 | `SYS_YIELD` | Yield CPU timeslice |
| 9 | `SYS_SLEEP` | Sleep for N timer ticks |
| 10 | `SYS_MALLOC` | Allocate kernel memory |
| 11 | `SYS_FREE` | Free kernel memory |
| 12 | `SYS_TIME` | Read RTC system time |
| 13 | `SYS_CLEAR` | Clear display |
| 14 | `SYS_UPTIME` | Read uptime seconds |
| 19 | `SYS_REBOOT` | Hardware reboot |
| 20 | `SYS_BEEP` | Audio beep |
