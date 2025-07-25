// This code only wraps the real syscall_handler
// to properly save and restore registers
// because the compiler always add prologue

.code16

.global isr

isr:
    cli
    pushw %ax
    pushw %bx
    pushw %cx
    pushw %dx
    pushw %si
    pushw %di

    movw %sp, %ax     // Parameter: char* stack_ptr
    pushw %ax
    call syscall_handler

    addw $2, %sp      // Remove parameter from stack

    popw %di
    popw %si
    popw %dx
    popw %cx
    popw %bx
    popw %ax

    sti
    iret
