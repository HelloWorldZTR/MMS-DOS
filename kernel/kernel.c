/* kernel.c */
#include "type.h"
#include "display.h"
#include "keyboard.h"
#include "lib.h"
#include "disk.h"

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
    puts("Attempt to read disk to heap...\n");
    bool ret = read_sector((heap_pointer(0)), 0, 0, 1, 0, 1); //read sector 1 
    if (ret) puts("Disk read failed!\n");

    fat12header fat12header;
    uint8_t* ptr = (uint8_t*)&fat12header;
    for(size_t i = 0; i < sizeof(fat12header); i++) {
        ptr[i] = heap_read_byte(i);
    }
    putchar('\n');
    for(size_t i = 0; i < 10; i++) {
        printf("%x ", ptr[i]);
    }
    putchar('\n');

    printf("Bytes per sector: %d\n", fat12header.BPB_BytePerSec);
    printf("Disk OEM is %s\n", (unsigned char*)fat12header.BS_OEMName);


    char cmd[100];
    while(1) {
        printf("A:>");
        gets(cmd, 100);
        printf("You've inputed: %s\n", cmd);
    }

    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}    