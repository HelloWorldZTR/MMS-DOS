/* loader.c */
// This submodule read kernel file from FAT12 filesystem
// and load it to memory, then jump to the kernel entry point
// to boot into actual kernel.
#include "type.h"
#include "display.h"
#include "lib.h"
#include "disk.h"

char KERNEL_NAME[] = "KERNEL  SYS";

extern void start_kernel(); // Kernel entry point

// Freeze the system 
void halt() {
    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}

// Load FAT table to heap 0x90000
void load_fat_table(fat12header header, uint16_t sector_num) {
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

void load_kernel(fat12header header,uint16_t first_cluster) {
    puts("Loading kernel...\n");

    // uint16_t physical_addr = 0x4200 + (first_cluster-2) * 512;
    // uint16_t sector = physical_addr / 512;
    uint16_t first_data_sector = header.BPB_RsvdSecCnt + (header.BPB_NumFATs * header.BPB_FATSz16) + header.BPB_RootEntCnt * sizeof(rootentry) / 512;
    
    uint16_t cur_cls = first_cluster;
    uint16_t next_cls;
    printf("First data sector: 0x%d\n", first_data_sector);
    for(size_t i = 0; i < 128; i++) { // max 64kb of file
        printf("Reading cluster: 0x%x\n", cur_cls);
        far_ptr dest = {
            .segment = 0x2000,
            .offset = i * 512
        };
        // Convert cluster number to sector number (CHS)
        uint16_t cur_sector = first_data_sector + (cur_cls - 2) * header.BPB_SecPerClus + 1;
        uint16_t track = cur_sector / 18;
        uint16_t cylinder = track / 2;
        uint16_t head = track % 2;
        uint16_t sector = cur_sector % 18;
        bool ret = read_sector(dest, 0, cylinder, head, sector, 1);
        if (ret) {
            printf("Failed to read cluster! 0x%x\n", ret);
            return;
        }

        next_cls = heap_read16(cur_cls * 3 / 2);
        if (cur_cls % 2 == 0) {
            next_cls = next_cls & 0x0FFF; // Even cluster number
        } else {
            next_cls = (next_cls >> 4) & 0x0FFF; // Odd cluster number
        }
        if (next_cls == 0xFFF) {
            puts("Kernel loaded!\n");
            start_kernel(); // Jump to kernel entry point
            return;
        }
        cur_cls = next_cls;
    }
    puts("Failed to read kernel file!\n");
    return;
}

void init_fs() {
    // Load header into heap
    puts("Loading Disk...\n");
    bool ret = read_sector(heap_pointer(0), 0, 0, 0, 1, 1); //read sector 1 
    if (ret) puts("Disk load failed!\n");

    fat12header header;
    uint8_t* ptr = (uint8_t*)&header;
    heap2memcpy(ptr, heap_pointer(0), sizeof(header));

    // printf("OEM Name: %s\n", header.BS_OEMName);

    // for(size_t i=0; i<10; i++) {
    //     printf("%x ", ptr[i]);
    // }
    // putchar('\n');s
    size_t bytes_per_sector = header.BPB_BytePerSec;
    size_t sectors_per_cluster = header.BPB_SecPerClus;
    size_t sectors_per_track = header.BPB_SecPerTrk;

    size_t reserved_sector_count = header.BPB_RsvdSecCnt;
    size_t fat_table_count = header.BPB_NumFATs;
    size_t fat_table_sector_size = header.BPB_FATSz16;
    size_t root_entry_count = header.BPB_RootEntCnt;

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
            return;
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
            load_fat_table(header, reserved_sector_count+1);
            load_kernel(header, entry.DIR_FstClus);
            return;
        }
    }
}


/* Entry point for the kernel */
void loader_main() {
    setcolor(WHITE, BLACK); // Set foreground and background colors
    puts("Booting MMS-DOS...\n");

    init_fs();

    halt();
}    