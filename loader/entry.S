.code16
.section .text
.global _start

.section .text
_start:
    call loader_main
    hlt

.global start_kernel
start_kernel:
    movw $0x2000, %ax
    movw %ax, %ss
    movw $0, %sp
    ljmp $0x2000, $0x0000 