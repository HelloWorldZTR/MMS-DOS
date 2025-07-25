#include "syscall.h"
#include "type.h"
#include "display.h"
#include "keyboard.h"
#include "lib.h"
#include "disk.h"

/**
 * @brief a wrapper for irq handler in isr.S
 * which add a wrapper to store and restore registers
 */
extern void isr();

/**
 * @brief Realll system call handler
 * @param stack_ptr sp
 * @note This is real system call handler
 */
void syscall_handler(uint16_t* stack_ptr){
    uint16_t reg_di = stack_ptr[0];
    uint16_t reg_si = stack_ptr[1];
    uint16_t reg_dx = stack_ptr[2];
    uint16_t reg_cx = stack_ptr[3];
    uint16_t reg_bx = stack_ptr[4];
    uint16_t reg_ax = stack_ptr[5];
    uint16_t reg_ds;
    asm volatile(
        "movw %%ds, %%ax\n"
        "movw %%ax, %[ds]\n"
        : [ds] "=r"(reg_ds)
        :
    );
    // All this *below* will be write back
    // not ax, bx, cx, dx
    uint8_t ah = reg_ax >> 8, al = reg_ax & 0xFF;
    uint8_t bh = reg_bx >> 8, bl = reg_bx & 0xFF;
    uint8_t ch = reg_cx >> 8, cl = reg_cx & 0xFF;
    uint8_t dh = reg_dx >> 8, dl = reg_dx & 0xFF;

    printf("Syscall with (AH:0x%x, AL:0x%x, BH:0x%x, BL:0x%x, CH:0x%x, CL:0x%x, DH:0x%x, DL:0x%x)",
           ah, al, bh, bl, ch, cl, dh, dl);
    switch (ah)
    {
    case SYS_PRINT_CHAR:
        // Al = char to print
        putchar(al);
        break;
    case SYS_PRINT_STRING:
        // DS:DX = string address, terminated by '\0'
        { // This block resticts the scope of defined var str_ptr
          // Or the compiler will be unhappy
            far_ptr str_ptr = {
                .segment = reg_ds,
                .offset = reg_cx
            };
            char cur = far_ptr_read(str_ptr);
            while (cur != '\0') {
                putchar(cur);
                str_ptr.offset++;
                cur = far_ptr_read(str_ptr);
            }
        }
        break;
    case SYS_READ_CHAR:
        // AL = keycode
        al = getchar();
        break;
    case SYS_READ_STRING:
        {
            far_ptr buf_ptr = {
                .segment = reg_ds,
                .offset = reg_cx
            };
            uint8_t max_len = far_ptr_read(buf_ptr);
            char buf[256]; // 0xFF is maximum max_len
            gets(buf, max_len);
            mem2heapcpy(buf_ptr, (near_ptr)buf, max_len);
        }
    case SYS_EXIT:
        // AL = exit code
        // We can use stack_ptr[6](ip) and stack_ptr[7](cs) to return to shell
        break;
    default:
        break;
    }
    // Write back registers
    reg_ax = (ah << 8) | al;
    reg_bx = (bh << 8) | bl;
    reg_cx = (ch << 8) | cl;
    reg_dx = (dh << 8) | dl;
    stack_ptr[5] = reg_ax;
    stack_ptr[4] = reg_bx;
    stack_ptr[3] = reg_cx;
    stack_ptr[2] = reg_dx;
    stack_ptr[1] = reg_si;
    stack_ptr[0] = reg_di;
}


/**
 * @brief Install isr from isr.S to handle int 0x21
 */
void install_interrupt_handlers() {
    uint16_t handler_offset = (uint16_t)(void *)isr;
    uint16_t handler_segment = 0x2000;
    asm volatile(
        "cli\n"
        "pushw %%es\n"
        "xor %%ax, %%ax\n"
        "movw %%ax, %%es\n"
        "movw $0x0084, %%di\n"          // Write offset to IDT int 21h Low
        "movw %[offset], %%ax\n"
        "movw %%ax, %%es:(%%di)\n"
        "movw $0x0086, %%di\n"          // Write segment to IDT int 21h High
        "movw %[segment], %%ax\n"
        "movw %%ax, %%es:(%%di)\n"
        "popw %%es\n"
        "sti\n"
        :
        : [offset] "r"(handler_offset), [segment] "r"(handler_segment)
        : "ax", "di"
    );
}