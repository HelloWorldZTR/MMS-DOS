/* kernel.c */
#include "type.h"
#include "display.h"
#include "keyboard.h"
#include "lib.h"

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

    puts("Attempt to write to 0x90000...\n");
    far_pointer fp = {0x9000, 0x0000};
    far_pointer_write(fp, 'M');
    printf("Far ptr (0x90000) is %c\n", far_pointer_read(fp));
    printf("Heap address 0x0 is %c\n", heap_read(0));

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