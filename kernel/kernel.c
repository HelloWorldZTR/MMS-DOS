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

void ls() {
    rootentry entry;

    for(size_t i = 0; i < fat_header.BPB_RootEntCnt; i++) {
        far_ptr entry_ptr = root_entry_ptr;
        entry_ptr.offset += i * sizeof(rootentry);
        heap2memcpy((uint8_t*)&entry, entry_ptr, sizeof(rootentry));

        if(entry.DIR_Name[0] == 0x00) {
            break; // End of entries
        }
        // Print the entry
        printf("- ");
        for(size_t j=0; j<11; j++) {
            putchar(entry.DIR_Name[j]);
        }
        if (entry.DIR_Attr & 0x10) {
            printf("<DIR>");
        }
        else {
            printf("\t %dB", entry.DIR_FileSize);
        }
        printf(" 0x%x", entry.DIR_FstClus);
        putchar('\n');
    }
}

void cd() {
    bool ret = read_fat_cls(root_entry_ptr, 0, 0x16);
    if(ret) puts("Failed!\n");
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

    while(1) {
        printf("A:/>");
        char input[256];
        gets(input, 256);
        if (strcmp(input, "ls") == 0) {
            ls();
        } else if (strcmp(input, "cd") == 0) {
            cd();
        } else if (strcmp(input, "halt") == 0) {
            puts("Halting system...\n");
            halt();
        } else {
            puts("Unknown command!\n");
        }

        putchar('\n');
    }

    halt(); // Halt the system
}    