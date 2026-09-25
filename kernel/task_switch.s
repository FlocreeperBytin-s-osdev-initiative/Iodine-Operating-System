.intel_syntax noprefix
.code32

.global switch_task

.section .text
switch_task:
    /* Push callee-saved registers and eflags */
    pushfd
    push ebp
    push ebx
    push esi
    push edi

    /* Load parameters */
    mov eax, [esp + 24]  /* prev_esp pointer */
    mov edx, [esp + 28]  /* next_esp value */

    /* Save old stack pointer */
    mov [eax], esp

    /* Switch to new stack pointer */
    mov esp, edx

    /* Pop registers from new stack */
    pop edi
    pop esi
    pop ebx
    pop ebp
    popfd

    ret

.section .note.GNU-stack,"",@progbits
