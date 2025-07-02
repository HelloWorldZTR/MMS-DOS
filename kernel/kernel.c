/* kernel.c */
#include "type.h"
#include "display.h"
#include "lib.h"
#include "disk.h"

/* Entry point for the kernel */
void kernel_main() {
    setcolor(WHITE, BLACK); // Set foreground and background colors

    clear();
    
    puts("Hi! Welcome to MMS-DOS!\n");

    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}    