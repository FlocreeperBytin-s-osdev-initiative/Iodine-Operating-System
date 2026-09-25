.intel_syntax noprefix
.code16
.global _start

.set KERNEL_SECTORS, 240 /* 240 sectors = 120 KiB */
.set KERNEL_LOAD_SEG, 0x1000

_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    /* Print loading message */
    mov si, offset msg_loading
    call print_string

    /* Load kernel sectors using BIOS INT 13h */
    mov ax, KERNEL_LOAD_SEG
    mov es, ax
    xor bx, bx          /* ES:BX = 0x1000:0000 */

    mov word ptr [current_lba], 1

read_sector_loop:
    mov ax, [current_lba]
    cmp ax, KERNEL_SECTORS + 1
    jge read_finished

    /* Convert LBA to CHS:
       LBA in AX
       Sectors per track = 18
       Heads = 2 */
    xor dx, dx
    mov cx, 18
    div cx              /* AX = LBA / 18, DX = LBA % 18 */
    inc dx              /* Sector = (LBA % 18) + 1 */
    mov cl, dl          /* CL = sector number (1-18) */

    xor dx, dx
    mov bx, 2
    div bx              /* AX = Cylinder, DX = Head */
    mov ch, al          /* CH = cylinder (0-79) */
    mov dh, dl          /* DH = head (0-1) */
    mov dl, [boot_drive]/* DL = drive */

    /* Buffer address in ES:DI */
    mov bx, [buf_offset]
    mov ah, 0x02        /* Read sectors */
    mov al, 1           /* 1 sector */
    int 0x13
    jc disk_read_error

    /* Advance buffer pointer */
    add word ptr [buf_offset], 512
    jnc next_sector
    /* If offset wrapped over 64K, increment ES by 0x1000 */
    mov ax, es
    add ax, 0x1000
    mov es, ax

next_sector:
    inc word ptr [current_lba]
    jmp read_sector_loop

read_finished:
    /* Enable A20 line via port 0x92 */
    in al, 0x92
    or al, 2
    out 0x92, al

    /* Disable interrupts for Protected Mode switch */
    cli

    /* Load Global Descriptor Table */
    lgdt [gdt_descriptor]

    /* Set PE bit in CR0 */
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    /* Far jump to 32-bit Code Segment */
    jmp 0x08:pm_start

disk_read_error:
    mov si, offset msg_disk_err
    call print_string
hang:
    hlt
    jmp hang

print_string:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

msg_loading:
    .asciz "Booting Iodine OS...\r\n"
msg_disk_err:
    .asciz "Disk read failure!\r\n"
boot_drive:
    .byte 0
current_lba:
    .word 0
buf_offset:
    .word 0

.align 4
gdt_start:
    .quad 0x0000000000000000 /* 0x00: Null descriptor */
    .quad 0x00CF9A000000FFFF /* 0x08: 32-bit Kernel Code (0 - 4GB) */
    .quad 0x00CF92000000FFFF /* 0x10: 32-bit Kernel Data (0 - 4GB) */
gdt_end:

gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long gdt_start

.code32
pm_start:
    /* Set up 32-bit segment registers */
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    /* Copy kernel from 0x10000 to 0x100000 (1MB) */
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, (KERNEL_SECTORS * 512) / 4
    cld
    rep movsd

    /* Set multiboot magic and jump to kernel at 0x100000 */
    mov eax, 0x2BADB002
    xor ebx, ebx
    jmp 0x100000

/* MBR Signature */
.org 510
.word 0xAA55
