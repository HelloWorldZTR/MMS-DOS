; bootloader.asm

org 0x7C00 
[bits 16]

section .data
    BOOT_DRIVE: db 0


section .text
start:
    ; Read boot drive from BIOS
    mov [BOOT_DRIVE], dl 

    ; Init Segment Registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
  
    ; Set up the stack
    mov ax, 0x9000
    mov ss, ax                  ; Set stack segment to 0x9000
    mov sp, 0xFFFF              ; Set stack pointer to the top of the stack

    ; Load the kernel from disk to 0x1000:0000
    call load_kernel

    ; Call kernel_main
    jmp 0x1000:0000             ; Jump to the kernel entry point

load_kernel:
    ; Load the kernel from disk to 0x1000:0000
    mov ax, 0x1000              ; Load the kernel segment address
    mov es, ax
    xor bx, bx                  ; Set BX to 0 (offset) 
                                ; Destination ES:BX 0x1000 0000

    mov ah, 0x02                ; read sectors
    mov al, 4                   ; read 4 sectors
    mov ch, 0                   ; cylinder  0
    mov cl, 2                   ; sector    2 (first sector after boot sector)
    mov dh, 0                   ; head      0
    mov dl, [BOOT_DRIVE]        ; drive     BOOT_DRIVE

    int 0x13
    mov al, "D"                 ; Disk Error
    jc load_failed

    ret

load_failed:
    mov ax, 0x0E
    int 0x10                    ; BIOS interrupt to print character in al
    hlt


times 510 - ($ - $$) db 0       ; Fill the rest of the boot sector to 512 bytes
dw 0xAA55                       ; Boot signature

