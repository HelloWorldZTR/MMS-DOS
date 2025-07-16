/* kernel.c */
/* This is the real kernel for MMS-DOS. It (therotically) sets up everthing
    and starts the shell and other drivers, if any.
*/
#include "type.h"
#include "display.h"
#include "keyboard.h"
#include "lib.h"
#include "disk.h"

/* Entry point for the kernel */
void kernel_main() {
    setcolor(WHITE, BLACK); // Set foreground and background colors

    clear();

    puts("Hi! Welcome to MMS-DOS!\n");

    while(1) {
        printf("A:/>");
        char input[256];
        gets(input, 256);
        printf("You entered: %s", input);

        putchar('\n');
    }

    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}    