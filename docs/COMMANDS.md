# Iodine OS - Shell & Command Reference

The **Iodine Shell (`ish`)** is the primary interactive command line interface of Iodine OS.

---

## 1. Line Editor & Shortcuts

- **Enter / Return**: Execute current command line.
- **Backspace**: Delete character before cursor.
- **Left / Right Arrow**: Navigate cursor horizontally within the input line.
- **Up / Down Arrow**: Cycle through command history (recalls previous commands).
- **Tab**: Auto-complete command names.
- **Ctrl + L**: Clear the console screen and redraw the current prompt.
- **Ctrl + C**: Cancel the current input line.

---

## 2. Package Management (`apk` / `ipk`)

Iodine OS includes an Alpine `apk` clone for package management:

```bash
apk update             # Update repository indexes
apk list               # List all available and installed packages
apk info [package]     # Show package details, license, and file list
apk add <package>      # Install package into Iodine OS VFS
apk del <package>      # Uninstall package from system
apk search <keyword>   # Search repository catalog
apk upgrade            # Upgrade installed packages
```

---

## 3. BSD Utilities (`bsdutils`)

Classic BSD core utilities integrated into Iodine OS:

```bash
cal [month] [year]     # Display BSD monthly calendar
hexdump -C <file>      # Canonical BSD hexadecimal + ASCII dump
column [words...]      # Format items into aligned columns
banner [text...]       # Print large retro ASCII billboard banner
morse [text...]        # Encode text into Morse code
cksum <file...>        # Compute POSIX / BSD 32-bit CRC checksum
whoami                 # Display current effective user (root)
```

---

## 4. Subsystem & Driver Controls

```bash
posix                  # Verify POSIX open, write, read, stat, lseek, brk
musl                   # Verify musl libc compatibility and Linux syscall translation
virtio                 # Probe and display VirtIO hardware devices on PCI bus
ldk [status|start|stop]# Manage Linux Driver Sandbox (GPL Contamination Barrier)
```

---

## 5. System Information & Diagnostics

```bash
fetch / neofetch       # Display ASCII art system banner & specifications
uname [-a]             # Print kernel version and host machine name
uptime                 # Show system running time and total ticks
date                   # Read hardware RTC date and time
mem / free             # RAM and heap utilization with ASCII bar
ps                     # Snapshot of active processes and threads
top                    # Interactive dynamic real-time task monitor
dmesg                  # Dump kernel boot message buffer
reboot                 # Soft reboot via 8042 keyboard controller
shutdown               # Power off machine via ACPI/APM
```

---

## 6. Filesystem Operations

```bash
ls [-l] [path]         # List directory contents
cd <path>              # Change working directory (.., ~, /)
pwd                    # Print current working directory
cat <path>             # Output file contents
touch <path>           # Create empty file
write <path> <text...> # Write string directly into file
mkdir <path>           # Create new directory
rm <path>              # Remove file or empty directory
wc <path>              # Count lines, words, and characters
```

---

## 7. Applications & Audio

```bash
snake                  # Play retro arcade Snake game (WASD / Arrow keys)
edit <path>            # Full-screen Nano-style visual text editor (^S save, ^Q quit)
matrix                 # Digital green Matrix rain screensaver
calc <expression>      # Math expression solver: (15 + 35) * 4
beep [freq] [duration] # Acoustic tone via PC speaker
melody                 # Play 8-bit fanfare theme on PC speaker
color <fg> [bg]        # Change console color palette
clear                  # Clear console screen
```
