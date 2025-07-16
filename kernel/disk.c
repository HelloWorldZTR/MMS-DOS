#include "disk.h"
#include "lib.h"
#include "display.h"

fat12header fat_header;
far_ptr root_entry_ptr;
far_ptr fat_table_ptr;

bool read_sector(far_ptr dest, size_t disknum, size_t cylindernum, size_t headnum, size_t sectornum) {
    uint16_t seg = dest.segment;
    uint16_t off = dest.offset;
    bool fail = false;
    uint16_t cylsec = cylindernum << 8 | sectornum;
    uint16_t headdisk = headnum << 8 | disknum;
    // uint8_t cnt = 0xFF & n;
    // asm volatile (
    //     "movb %[cnt], %%al\n"
    //     : 
    //     : [cnt] "r"(cnt)
    //     :
    // );
    asm volatile (
        "movw %%ax, %%es\n"
        "movb $0x02, %%ah\n"
        "movb $0x01, %%al\n"
        "int $0x13\n"   // BIOS disk interrupt
        "jc 1f\n" 
        "movw $0, %[fail]\n"
        "jmp 2f\n"
        "1:\n"
        "movw $1, %[fail]\n"
        "2:\n"
        : [fail] "=r"(fail)
        : "a"(seg), "b"(off), "c"(cylsec), "d"(headdisk)
        :
    );
    return fail;
}

bool read_fat_header(size_t disknum) {
    far_ptr tmp_ptr = {
        .segment = 0x1000,
        .offset = 0
    }; 
    bool ret = read_sector(tmp_ptr, disknum, 0, 0, 1);
    
    if (ret) {
        puts("Failed to read FAT header!\n");
    }
    uint8_t* ptr = (uint8_t*)&fat_header;
    heap2memcpy(ptr, tmp_ptr, sizeof(fat_header));
    return ret;
}

bool load_fat_table(size_t disknum) {
    bool ret = false;

    size_t bytes_per_sector = fat_header.BPB_BytePerSec;
    size_t sectors_per_cluster = fat_header.BPB_SecPerClus;
    size_t sectors_per_track = fat_header.BPB_SecPerTrk;
    size_t reserved_sector_count = fat_header.BPB_RsvdSecCnt;
    size_t fat_table_count = fat_header.BPB_NumFATs;
    size_t fat_table_sector_count = fat_header.BPB_FATSz16;
    size_t root_entry_count = fat_header.BPB_RootEntCnt;

    size_t rootentry_pos = reserved_sector_count + (fat_table_count * fat_table_sector_count) + 1;
    size_t rootentry_length = root_entry_count * sizeof(rootentry);
    size_t entry_track = rootentry_pos / sectors_per_track;
    size_t entry_sector = rootentry_pos % sectors_per_track;
    size_t entry_cylinder = entry_track / 2;
    size_t entry_head = entry_track % 2;

    size_t fat_table_pos = reserved_sector_count + 1;
    size_t fat_table_track = fat_table_pos / 18;
    size_t fat_table_cylinder = fat_table_track / 2;
    size_t fat_table_head = fat_table_track % 2;
    size_t fat_table_sector = fat_table_pos % 18;

    fat_table_ptr.segment = 0x1000;
    fat_table_ptr.offset = 0x0000;
    ret = read_sector(fat_table_ptr, disknum, fat_table_cylinder, fat_table_head, fat_table_sector);
    if (ret) {
        puts("Failed to read FAT table sector!\n");
        return ret;
    }
    
    root_entry_ptr.segment = 0x1000;
    root_entry_ptr.offset = fat_table_sector_count * bytes_per_sector;
    ret = read_sector(root_entry_ptr, disknum, entry_cylinder, entry_head, entry_sector);
    if (ret) {
        puts("Failed to read root entry sector!\n");
        return ret;
    }

    return ret;
}