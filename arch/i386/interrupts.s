.intel_syntax noprefix
.code32

.global idt_flush
idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    ret

/* Macros for ISR handlers */
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    push 0          /* Dummy error code */
    push \num       /* Interrupt number */
    jmp isr_common_stub
.endm

.macro ISR_ERRCODE num
.global isr\num
isr\num:
    push \num       /* Interrupt number (error code already on stack) */
    jmp isr_common_stub
.endm

.macro IRQ num, irq_num
.global irq\num
irq\num:
    push 0          /* Dummy error code */
    push \irq_num   /* Interrupt number (32 + num) */
    jmp irq_common_stub
.endm

/* CPU Exceptions */
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE   30
ISR_NOERRCODE 31

/* Syscall ISR 128 (0x80) */
ISR_NOERRCODE 128

/* IRQs 0 - 15 */
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

.extern isr_handler
.extern irq_handler

isr_common_stub:
    pusha           /* Pushes edi, esi, ebp, esp, ebx, edx, ecx, eax */

    mov ax, ds
    push eax        /* Save data segment */

    mov ax, 0x10    /* Kernel data segment */
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp        /* Pass registers_t pointer to isr_handler */
    call isr_handler
    add esp, 4

    pop eax         /* Restore data segment */
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa            /* Restores edi, esi, ebp, esp, ebx, edx, ecx, eax */
    add esp, 8      /* Cleans up pushed error code and ISR number */
    iret

irq_common_stub:
    pusha

    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iret

.section .note.GNU-stack,"",@progbits
