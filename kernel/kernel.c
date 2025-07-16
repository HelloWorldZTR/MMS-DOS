/* kernel.c */
/* This is the real kernel for MMS-DOS. It (therotically) sets up everthing
    and starts the shell and other drivers, if any.
*/
#include "type.h"
#include "display.h"
#include "keyboard.h"
#include "lib.h"
#include "disk.h"

void halt() {
    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}

/* Entry point for the kernel */
void kernel_main() {
    setcolor(WHITE, BLACK); // Set foreground and background colors

    clear();

    puts("Hi! Welcome to MMS-DOS!\n");

    bool ret = false;
    ret |= read_fat_header(0);
    ret |= load_fat_table(0);
    if (ret) {
        puts("Failed to initialize FS!\n");
        halt();
    }

    // printf("FAT Header: %s\n", fat_header.BS_OEMName);
    // for(size_t i = 0; i < fat_header.BPB_RootEntCnt; i++) {
    //     rootentry entry;
    //     far_ptr entry_ptr = root_entry_ptr;
    //     entry_ptr.offset += i * sizeof(rootentry);
    //     heap2memcpy((uint8_t*)&entry, entry_ptr, sizeof(rootentry));

    //     if(entry.DIR_Name[0] == 0x00) {
    //         break; // End of entries
    //     }

    //     printf("- %s\n", entry.DIR_Name);
    // }

    while(1) {
        printf("A:/>");
        char input[256];
        gets(input, 256);
        printf("You entered: %s", input);

        putchar('\n');
    }

    halt(); // Halt the system
}    