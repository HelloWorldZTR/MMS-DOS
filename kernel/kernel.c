/* kernel.c */
#include "type.h"
#include "display.h"
#include "keyboard.h"

void print_banner() {
    for (int i = 0; i < 30; i++) putchar('=');
    puts("\n\tWelcome to MMS-DOS!\n");
    for (int i = 0; i < 30; i++) putchar('=');
    putchar('\n');
}

/* Entry point for the kernel */
void kernel_main() {
    setcolor(WHITE, BLACK); // Set foreground and background colors
    print_banner();

    char cmd[100];
    while(1) {
        printf("A:>");
        // gets(cmd, 100);
        int t = getint();
        printf("You've inputed: %d\n", t);
    }

    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}    