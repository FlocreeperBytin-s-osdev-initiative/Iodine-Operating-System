# Iodine OS - Kernel & System Architecture

**Iodine OS** is an independent, monolithic, 32-bit and 64-bit x86 operating system written from scratch in C and x86 Assembly. It contains **no code or dependencies from Linux, BSD, Unix, or any existing distribution**.

---

## 1. 64-Bit Architecture & Long Mode Support (`arch/x86_64/`)

Iodine OS features full 64-bit long-mode architecture support:
- **64-Bit Types & Data Models (`arch/x86_64/types64.h`)**: `uint64_t`, `int64_t`, `size_t` (64-bit), and `uintptr_t` (64-bit) under the LP64 data model.
- **4-Level Paging Memory Management (`arch/x86_64/mmu.h`)**:
  - **PML4** (Page Map Level 4): 512 entries controlling 512 GB virtual address spaces per entry.
  - **PDPT** (Page Directory Pointer Table): 512 entries controlling 1 GB spaces.
  - **PD** (Page Directory): 512 entries with 2MB huge page or 4KB page table mappings.
  - **PT** (Page Table): 512 entries mapping 4KB physical page frames.
  - Full support for canonical address sign-extension (`0x0000000000000000` to `0x00007FFFFFFFFFFF` and `0xFFFF800000000000` to `0xFFFFFFFFFFFFFFFF`).
- **Fast 64-Bit System Calls (`arch/x86_64/syscall64.h`)**:
  - `MSR_EFER` (0xC0000080): SCE (bit 0) enabled for fast `syscall` and `sysret` instruction execution.
  - `MSR_STAR` (0xC0000081): Kernel and user code/data segment selectors.
  - `MSR_LSTAR` (0xC0000082): Target 64-bit RIP for userspace `syscall` entry.
  - `MSR_SFMASK` (0xC0000084): Automatically masks `RFLAGS` interrupts on entry.

---

## 2. POSIX Compliance & Subsystem (`compat/posix/`)

Iodine OS provides a native POSIX standard compliance layer:
- **File Descriptors**: Full 0..63 file descriptor table managing stdin (0), stdout (1), stderr (2), and opened files/devices.
- **POSIX Operations**:
  - `posix_open(path, flags, mode)` with `O_RDONLY`, `O_WRONLY`, `O_RDWR`, `O_CREAT`, `O_TRUNC`, `O_APPEND`.
  - `posix_read(fd, buf, count)` and `posix_write(fd, buf, count)`.
  - `posix_close(fd)`.
  - `posix_lseek(fd, offset, whence)` with `SEEK_SET`, `SEEK_CUR`, `SEEK_END`.
  - `posix_stat(path, buf)` and `posix_fstat(fd, buf)` mapping VFS nodes to `struct posix_stat`.
  - `posix_dup(oldfd)` and `posix_dup2(oldfd, newfd)`.
  - `posix_mkdir(path, mode)` and `posix_unlink(path)`.
  - `posix_brk(addr)` managing user heap boundaries.
  - `posix_mmap(addr, length, prot, flags, fd, offset)` and `posix_munmap(addr, length)`.
- **POSIX Standard Error Codes (`compat/posix/errno.h`)**: Full suite of standard codes (`ENOENT`, `EACCES`, `EBADF`, `ENOMEM`, `EEXIST`, `EINVAL`, `ENOSYS`, etc.).

---

## 3. Linux Syscall Translator (`compat/linux/`)

Similar to Windows WSL1, FreeBSD's Linuxulator, and Solaris's lx-brand zones, Iodine OS features an in-kernel **Linux Syscall Translation Layer**:
- Intercepts standard Linux system call numbers and ABI structures and maps them to Iodine's native POSIX/VFS subsystems.
- Supported Linux System Calls:
  - `sys_read` (0), `sys_write` (1), `sys_open` (2), `sys_close` (3)
  - `sys_stat` (4), `sys_fstat` (5), `sys_lseek` (8)
  - `sys_mmap` (9), `sys_mprotect` (10), `sys_munmap` (11), `sys_brk` (12)
  - `sys_ioctl` (16), `sys_writev` (20), `sys_access` (21)
  - `sys_dup` (32), `sys_dup2` (33), `sys_getpid` (39)
  - `sys_fork` (57), `sys_exit` (60), `sys_uname` (63)
  - `sys_getcwd` (79), `sys_chdir` (80), `sys_mkdir` (83), `sys_unlink` (87)
  - `sys_gettimeofday` (96), `sys_getuid` (102), `sys_clock_gettime` (228), `sys_exit_group` (231)
- Binary Data Structure Translation:
  - Translates between `struct linux_stat64` and Iodine's `vfs_stat`.
  - Translates `struct linux_utsname`, `struct linux_timespec`, `struct linux_timeval`, and `struct linux_iovec`.
  - Error returns cleanly formatted as `-errno`.

---

## 4. musl libc Compatibility Runtime (`compat/musl/`)

- Target ABI: `musl-x86_64` and `musl-i386`.
- Direct routing of musl system calls through the Linux Translator.
- Validated with real musl ABI structs: `musl_sys_uname`, `musl_sys_getpid`, `musl_sys_clock_gettime`.

---

## 5. Trusty ol' VirtIO Subsystem (`drivers/virtio/`)

Iodine OS includes an integrated VirtIO driver framework:
- **PCI Bus Probe (`drivers/virtio/virtio_pci.c`)**: Scans PCI buses, discovers VirtIO devices (`Vendor ID 0x1AF4`), negotiates device features, and assigns I/O base ports and IRQ lines.
- **Split Virtqueue Architecture (`drivers/virtio/virtqueue.c`)**:
  - `struct vring_desc`: 16-byte descriptors (`addr`, `len`, `flags`, `next`).
  - `struct vring_avail`: Ring of descriptors offered to the host.
  - `struct vring_used`: Ring of descriptors processed by the host.
  - Memory structures aligned to 4096-byte boundaries per specification.
- **VirtIO Block Controller (`drivers/virtio/virtio_blk.c`)**: Fast sector I/O request queues.
- **VirtIO Network Adapter (`drivers/virtio/virtio_net.c`)**: Ethernet packet transfer and MAC address discovery.

---

## 6. Linux Driver Sandbox (LDK - GPL Contamination Barrier)

### Legal & Technical Rationale
Iodine OS is licensed under BSD 2-Clause. Linux drivers are typically licensed under GNU GPLv2. If GPL driver code were linked directly into the monolithic kernel binary, GPL contagion would legally infect the entire kernel.

### The Solution: Isolated Driver Domain
Located in `compat/linux_driver_sandbox/`:
1. **Isolated Execution Context**: Linux drivers execute inside a memory-protected sandbox domain (similar to user-mode driver frameworks or microkernel driver domains).
2. **Synthetic Linux Kernel Environment (`linux_env.c`)**: Emulates the Linux internal driver API:
   - `struct pci_dev`, `pci_register_driver()`
   - `struct net_device`, `register_netdev()`
   - `struct sk_buff`, `netif_rx()`
   - `kmalloc()`, `kfree()`, `spin_lock()`, `printk()`
3. **Clean-Room Arms-Length IPC Bridge (`ldk_bridge.c`)**: Communication between the Iodine kernel and the driver sandbox occurs strictly over an IPC ring buffer passing message structures (`LDK_MSG_INIT`, `LDK_MSG_TX_PACKET`, `LDK_MSG_RX_PACKET`).
4. **Concrete Sandboxed Driver**: `drivers/linux_e1000.c` implements the Intel PRO/1000 driver using the Linux network driver API, running entirely within the sandbox and transmitting frames through the LDK bridge!
5. **Zero GPL Contamination**: The Iodine kernel binary contains zero GPL symbols and maintains 100% BSD 2-Clause licensing.

---

## 7. Alpine's apk Clone (`apps/apk/` / `ipk`)

Iodine OS features an Alpine Linux `apk` package manager clone:
- Commands: `apk add`, `apk del`, `apk list`, `apk info`, `apk search`, `apk update`, `apk upgrade`.
- Repository catalog featuring: `musl`, `bsdutils`, `virtio-tools`, `ldk-sandbox`, `curl`, `lua`, `busybox-posix`.
- Manages installation to VFS and package metadata.

---

## 8. bsdutils Suite (`apps/bsdutils/`)

Port of standard BSD utilities:
- `cal`: BSD monthly calendar generator with leap year and day-of-week calculations.
- `hexdump`: Canonical BSD hex + ASCII dumper (`hexdump -C <file>`).
- `column`: Aligned multi-column list formatting.
- `banner`: Large retro ASCII billboard banner printing.
- `morse`: Text to Morse code encoder.
- `cksum`: POSIX / BSD 32-bit CRC checksum generator.
- `whoami`: Effective user display.
