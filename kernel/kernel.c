/* kernel.c */
#include "type.h"
#include "display.h"

/* Entry point for the kernel */
void kernel_main() {
    setcolor(GREEN, BLACK); // Set foreground and background colors
    puts("Welcome to MMS-DOS!\n");
    puts("This is a very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long sentence.\n");

    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}