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

## 2. Command Index

### System Information & Diagnostics

#### `fetch` / `neofetch`
Displays the colorful ASCII art Iodine OS badge alongside host specs, kernel version, uptime, memory utilization, and hardware date.
```
iodine:/home/user# fetch
```

#### `uname [-a]`
Prints system and kernel information.
```
iodine:/home/user# uname -a
IodineOS iodine-pc 1.0.0 #1 SMP PREEMPT 2026 i386 Iodine
```

#### `uptime`
Reports how long the system has been running in hours, minutes, seconds, and total timer ticks.
```
iodine:/home/user# uptime
Uptime: 00:04:12 (Total Ticks: 25200)
```

#### `date`
Queries the hardware CMOS Real Time Clock and prints the UTC date and time.
```
iodine:/home/user# date
2026-09-25 21:00:00 UTC
```

#### `mem` / `free`
Displays detailed memory usage for physical RAM blocks and dynamic kernel heap, complete with an ASCII bar graph.
```
iodine:/home/user# mem
```

#### `dmesg`
Dumps the kernel boot message log buffer.
```
iodine:/home/user# dmesg
```

---

### Process & Task Management

#### `ps`
Lists all active processes, displaying PID, name, state (`READY`, `RUNNING`, `SLEEPING`, `ZOMBIE`), priority, and CPU runtime ticks.
```
iodine:/home/user# ps
```

#### `top`
Interactive dynamic real-time system monitor. Automatically refreshes every second showing CPU time, uptime, memory, and process states. Press any key (or `q`) to exit back to the shell.
```
iodine:/home/user# top
```

#### `kill <pid>`
Terminates a process by its process ID.
```
iodine:/home/user# kill 3
```

#### `sleep <seconds>`
Suspends execution for the specified number of seconds using timer interrupts.
```
iodine:/home/user# sleep 2
```

---

### File & Directory Management

#### `ls [-l] [path]`
Lists files and directories. Supports `-l` for detailed view with file types, permissions, and sizes.
```
iodine:/home/user# ls -l
-rw-r--r--    479  welcome.txt
-rw-r--r--    104  notes.txt
```

#### `cd <path>`
Changes current working directory. Supports `..`, `/`, `~`, and relative paths.
```
iodine:/home/user# cd /etc
iodine:/etc# cd ..
iodine:/#
```

#### `pwd`
Prints the current working directory path.
```
iodine:/home/user# pwd
/home/user
```

#### `cat <path>`
Reads and outputs the contents of a file to the screen.
```
iodine:/home/user# cat /etc/version
Iodine Operating System 1.0.0 (Release 2026)
```

#### `touch <path>`
Creates a new empty file in the filesystem.
```
iodine:/home/user# touch myfile.txt
```

#### `write <path> <text...>`
Writes text directly into a file.
```
iodine:/home/user# write notes.txt System is running flawlessly!
```

#### `mkdir <path>`
Creates a new directory in the filesystem.
```
iodine:/home/user# mkdir projects
```

#### `rm <path>`
Removes a file or empty directory from the filesystem.
```
iodine:/home/user# rm oldfile.txt
```

#### `hexdump <path>`
Displays a canonical hexadecimal and ASCII byte dump of any file.
```
iodine:/home/user# hexdump /etc/hostname
```

#### `wc <path>`
Counts the lines, words, and characters of a text file.
```
iodine:/home/user# wc welcome.txt
```

---

### Applications & Games

#### `snake`
Launches the full-featured retro arcade Snake game on the VGA text console!
- Controls: `W`, `A`, `S`, `D` or Arrow Keys.
- Objective: Eat `*` to grow and increase score.
- Sound effects via PC speaker.
- Press `Q` to quit.
```
iodine:/home/user# snake
```

#### `edit <path>`
Launches the full-screen visual text editor ("Iodine Edit"):
- Nano-style visual interface with top status bar and bottom shortcut bar.
- Arrow keys for cursor navigation.
- `Ctrl + S`: Save file to VFS.
- `Ctrl + Q`: Exit editor.
```
iodine:/home/user# edit mydocument.txt
```

#### `matrix`
Displays an animated green digital rain Matrix screen saver effect.
Press any key to return to the shell.
```
iodine:/home/user# matrix
```

#### `calc <expression>`
Evaluates mathematical expressions with operator precedence and parentheses.
Supported operators: `+`, `-`, `*`, `/`, `%`, `(`, `)`.
```
iodine:/home/user# calc (15 + 35) * 4
(15 + 35) * 4 = 200
```

---

### Hardware, Audio & Display

#### `beep [frequency] [duration_ms]`
Generates an acoustic audio tone through the PC speaker.
```
iodine:/home/user# beep 440 300
```

#### `melody`
Plays a classic 8-bit fanfare musical theme on the PC speaker!
```
iodine:/home/user# melody
```

#### `color <fg> [bg]`
Changes console foreground and background colors (0-15 VGA palette).
```
iodine:/home/user# color 10 0
```

#### `clear`
Clears the console display and resets cursor to top-left.
```
iodine:/home/user# clear
```

#### `reboot`
Safely reboots the computer via the 8042 keyboard controller reset line.
```
iodine:/home/user# reboot
```

#### `shutdown`
Powers down the computer via ACPI / APM hardware power ports.
```
iodine:/home/user# shutdown
```
