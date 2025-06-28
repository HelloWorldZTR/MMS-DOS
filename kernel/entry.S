.code16
.section .text
.global _start

.section .text
_start:
    call kernel_main
    hlt
