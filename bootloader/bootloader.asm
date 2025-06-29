; bootloader.asm

[bits 16]

header:
    jmp start
    nop

    BS_OEMName     db "MCROSOFT"
    BPB_BytsPerSec dw 512
    BPB_SecPerClus db 1
    BPB_RsvdSecCnt dw 9 ; 1 Boot + 8 Kernel
    BPB_NumFATs    db 2
    BPB_RootEntCnt dw 224
    BPB_TotSec16   dw 2880
    BPB_Media      db 0xF0
    BPB_FATSz16    dw 9
    BPB_SecPerTrk  dw 18
    BPB_NumHeads   dw 2
    BPB_HiddSec    dd 0
    BPB_TotSec32   dd 0
    BS_DrvNum      db 0
    BS_Reserved1   db 0
    BS_BootSig     db 0x29
    BS_VolID       dd 0
    BS_VolLab      db "MSS-DOS 0.1"
    BS_FileSysType db "FAT12   "

    BOOT_DRIVE: db 0


section .text
start:
    ; Read boot drive from BIOS
    mov [BOOT_DRIVE], dl 

    ; Enable Text Mode 
    mov ah, 0x00
    mov al, 0x03
    int 0x10

    ; Init Segment Registers
    xor ax, ax
    mov ds, ax
    mov es, ax

    ; Load the kernel from disk to 0x1000:0000
    call load_kernel

    ; Set up the stack
    mov ax, 0x1000
    mov ss, ax                  ; Set stack segment to 0x1000
    mov sp, 0xFFFF              ; Set stack pointer to the top of the stack

    ; Call kernel_main
    jmp 0x1000:0000             ; Jump to the kernel entry point
    mov al, "K"
    call load_failed

load_kernel:
    ; Load the kernel from disk to 0x1000:0000
    mov ax, 0x1000              ; Load the kernel segment address
    mov es, ax
    xor bx, bx                  ; Set BX to 0 (offset) 
                                ; Destination ES:BX 0x1000 0000

    mov ah, 0x02                ; read sectors
    mov al, 8                   ; read 8 sectors
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

