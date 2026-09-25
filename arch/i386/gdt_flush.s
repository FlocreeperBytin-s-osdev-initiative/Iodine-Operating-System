.intel_syntax noprefix
.code32

.global gdt_flush
.global tss_flush

.section .text
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    /* Far jump to flush CS to 0x08 */
    jmp 0x08:.flush
.flush:
    ret

tss_flush:
    mov ax, 0x2B /* Index 5 in GDT (5 * 8 = 40 = 0x28) with RPL 3 = 0x2B */
    ltr ax
    ret

.section .note.GNU-stack,"",@progbits
