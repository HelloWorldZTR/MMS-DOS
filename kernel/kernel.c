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



/* Entry point for the kernel */
void kernel_main() {
    setcolor(WHITE, BLACK); // Set foreground and background colors

    clear();

    puts("Hi! Welcome to MMS-DOS!\n");

    if (read_fat_header(0) || load_fat_table(0) || read_root_entry(0)) {
        puts("Failed to initialize FS!\n");
        halt();
    }

    shell_main(); // Start the shell

    halt(); // Halt the system
}    