/* kernel.c */
#include "type.h"
#include "display.h"
#include "lib.h"
#include "disk.h"

char KERNEL_NAME[] = "KERNEL  COM";

void print_banner() {
    for (int i = 0; i < 30; i++) putchar('=');
    puts("\n\tMMS-DOS 2nd stage bootloader\n");
    for (int i = 0; i < 30; i++) putchar('=');
    putchar('\n');
}

// Load FAT table to heap 0x90000
void load_fat_table(uint16_t sector_num) {
    uint16_t track = sector_num / 18; // 18 sectors per track
    uint16_t cylinder = track / 2;
    uint16_t head = track % 2;
    uint16_t sector = sector_num % 18;
    bool ret = read_sector(heap_pointer(0), 0, cylinder, head, sector, 1);
    if (ret) {
        puts("Failed to read FAT table sector!\n");
        return;
    }
}

void load_kernel(uint16_t first_cluster) {
    puts("Loading kernel...\n");

    // uint16_t physical_addr = 0x4200 + (first_cluster-2) * 512;
    // uint16_t sector = physical_addr / 512;
    uint16_t cur_cls = first_cluster;
    uint16_t next_cls;
    while(1) {
        printf("Read cluster: 0x%x\n", cur_cls);

        next_cls = heap_read16(cur_cls * 3 / 2);
        if (cur_cls % 2 == 0) {
            next_cls = next_cls & 0x0FFF; // Even cluster number
        } else {
            next_cls = (next_cls >> 4) & 0x0FFF; // Odd cluster number
        }
        if (next_cls == 0xFFF) {
            puts("Farewell!\n");
            break;
        }
        cur_cls = next_cls;
    }
}

void init_fs() {
    // Load header into heap
    puts("Loading Disk...\n");
    bool ret = read_sector(heap_pointer(0), 0, 0, 0, 1, 1); //read sector 1 
    if (ret) puts("Disk load failed!\n");

    fat12header fat12header;
    uint8_t* ptr = (uint8_t*)&fat12header;
    heap2memcpy(ptr, heap_pointer(0), sizeof(fat12header));

    // printf("OEM Name: %s\n", fat12header.BS_OEMName);

    // for(size_t i=0; i<10; i++) {
    //     printf("%x ", ptr[i]);
    // }
    // putchar('\n');s
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
        far_ptr entry_ptr = heap_pointer(i * sizeof(rootentry));
        heap2memcpy((uint8_t*)&entry, entry_ptr, sizeof(rootentry));

        if (entry.DIR_Name[0] == 0x00) {
            puts("Kernel not found.\n");
            break; // End of entries
        }

        bool found = true;
        for (size_t j = 0; j < 11; j++) {
            if (entry.DIR_Name[j] != KERNEL_NAME[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            puts("Found kernel file!\n");

            // uint8_t* entry_ptr = (uint8_t*)&entry;
            // for (size_t j = 0; j < 32; j++) {
            //     printf("%x ", entry_ptr[j]);
            // }
            // putchar('\n');
            // printf("First cluster %x \n", entry.DIR_FstClus);

            load_fat_table(reserved_sector_count+1);
            load_kernel(entry.DIR_FstClus);
            return;
        }

    }
}


/* Entry point for the kernel */
void kernel_main() {
    setcolor(WHITE, BLACK); // Set foreground and background colors
    print_banner();

    init_fs();

    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}    