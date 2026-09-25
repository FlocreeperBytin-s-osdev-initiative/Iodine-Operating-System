.intel_syntax noprefix
.code32

/* Multiboot 1 specification constants */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 32768 /* 32 KiB initial stack */
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    /* Set up stack */
    mov esp, offset stack_top

    /* Reset EFLAGS */
    push 0
    popf

    /* Push multiboot information onto stack */
    push ebx /* Multiboot info structure pointer */
    push eax /* Multiboot magic number */

    /* Enter kernel C main */
    call kernel_main

    /* If kernel_main returns, halt CPU */
    cli
halt_loop:
    hlt
    jmp halt_loop

.size _start, . - _start

.section .note.GNU-stack,"",@progbits
