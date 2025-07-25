/* kernel.c */
/* This is the real kernel for MMS-DOS. It (therotically) sets up everthing
    and starts the shell and other drivers, if any.
*/
#include "type.h"
#include "display.h"
#include "keyboard.h"
#include "lib.h"
#include "disk.h"
#include "shell.h"
#include "syscall.h"



/* Entry point for the kernel */
void kernel_main() {
    setcolor(WHITE, BLACK); // Set foreground and background colors

    clear();

    puts("Hi! Welcome to MMS-DOS!\n");

    if (read_fat_header(0) || load_fat_table(0) || read_root_entry(0)) {
        puts("Failed to initialize FS!\n");
        halt();
    }

    install_interrupt_handlers();

    // Test 0x02
    asm volatile(
        "movb %[syscall], %%ah\n"
        "movb $0x61, %%al\n" // A 
        "int $0x21\n" // Trigger interrupt 21h to test syscall handler
        :
        : [syscall] "i"(SYS_PRINT_CHAR)
    );
    putchar('\n');

    // Test 0x09
    char test_str[] = "\nSyscall has been installed!\n";
    uint16_t offset = (uint16_t) test_str;
    uint16_t segment = 0x2000; // Segment for kernel code
    asm volatile(
        "movw %[segment], %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %[offset], %%dx\n"
        "movb %[syscall], %%ah\n"
        "movb $0x61, %%al\n" // A 
        "int $0x21\n" // Trigger interrupt 21h to test syscall handler
        :
        : [syscall] "i"(SYS_PRINT_STRING), [offset] "r"(offset), [segment] "r"(segment)
    );
    
    // Test 0x01
    uint8_t keycode;
    asm volatile(
        "movb %[syscall], %%ah\n"
        "int $0x21\n" // Trigger interrupt 21h to test syscall handler
        "movb %%al, %[ret]\n"
        : [ret] "=r"(keycode)
        : [syscall] "i"(SYS_READ_CHAR)
    );
    printf("al: 0x%x\n", keycode);

    
    shell_main();

    halt(); // Halt the system
}    