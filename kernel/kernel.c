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

void init_fs() {
    // Load header into heap
    puts("Loading Disk...\n");
    bool ret = read_sector((heap_pointer(0)), 0, 0, 0, 1, 1); //read sector 1 
    if (ret) puts("Disk load failed!\n");

    fat12header fat12header;
    uint8_t* ptr = (uint8_t*)&fat12header;
    heap2memcpy(ptr, heap_pointer(0), sizeof(fat12header));

    // printf("OEM Name: %s\n", fat12header.BS_OEMName);

    // for(size_t i=0; i<10; i++) {
    //     printf("%x ", ptr[i]);
    // }
    // putchar('\n');
    size_t bytes_per_sector = fat12header.BPB_BytePerSec;
    size_t sectors_per_cluster = fat12header.BPB_SecPerClus;
    size_t sectors_per_track = fat12header.BPB_SecPerTrk;

    size_t reserved_sector_count = fat12header.BPB_RsvdSecCnt;
    size_t fat_table_count = fat12header.BPB_NumFATs;
    size_t fat_table_sector_size = fat12header.BPB_FATSz16;
    size_t root_entry_count = fat12header.BPB_RootEntCnt;

    size_t rootentry_pos = reserved_sector_count + (fat_table_count * fat_table_sector_size) + 1;
    size_t rootentry_length = root_entry_count * sizeof(rootentry);
    size_t entry_track = rootentry_pos / sectors_per_track;

    size_t entry_sector = rootentry_pos % sectors_per_track;
    size_t entry_cylinder = entry_track / 2;
    size_t entry_head = entry_track % 2;

    // printf("Root Entry at sector %d, equ to cylinder %d, head %d, sector %d\n",
    //    rootentry_pos, entry_cylinder, entry_head, entry_sector);
    ret = read_sector(heap_pointer(0), 0, entry_cylinder, entry_head, entry_sector, 1);
    if (ret) puts("Failed to read root entry sector!\n");

    for(size_t i = 0; i < root_entry_count; i++) {
        rootentry entry;
        far_pointer entry_ptr = heap_pointer(i * sizeof(rootentry));
        heap2memcpy((uint8_t*)&entry, entry_ptr, sizeof(rootentry));

        if (entry.DIR_Name[0] == 0x00) {
            puts("End of root entries reached.\n");
            break; // End of entries
        }

        printf("- %s \n", entry.DIR_Name);
        // uint8_t *t_ptr = (uint8_t*)&entry;
        // for(size_t i = 0; i < 10; i++) {
        //     printf("0x%x ", t_ptr[i]);
        // }
    }
}

void interpret_command(const char *cmd) {
    printf("Interpreting command: %s\n", cmd);
}

void init_shell() {
    char cmd[100];
    while(1) {
        printf("A:>");
        gets(cmd, 100);
        interpret_command(cmd); 
    }
}

/* Entry point for the kernel */
void kernel_main() {
    setcolor(WHITE, BLACK); // Set foreground and background colors
    print_banner();

    init_fs();

    init_shell();

    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}    