#include "disk.h"
#include "lib.h"
#include "display.h"

fat12header fat_header;
far_ptr root_entry_ptr = {// Max 4kb 0x11000 - 0x11FFF
    .segment = 0x1000,
    .offset = 0x1000
};
far_ptr fat_table_ptr = { // Max 4kb 0x10000 - 0x10FFF
    .segment = 0x1000,
    .offset = 0x0000
};

bool read_sector(far_ptr dest, size_t disknum, size_t cylindernum, size_t headnum, size_t sectornum) {
    uint16_t seg = dest.segment;
    uint16_t off = dest.offset;
    uint16_t cylsec = cylindernum << 8 | sectornum;
    uint16_t headdisk = headnum << 8 | disknum;

    uint8_t err;
    asm volatile (
        "movw %%ax, %%es\n"
        "movb $0x02, %%ah\n"
        "movb $0x01, %%al\n"
        "int $0x13\n"   // BIOS disk interrupt
        "jc 1f\n"
        "movb $0, %[err]\n"     // 成功：错误码设为 0
        "jmp 2f\n"
        "1:\n"
        "movb %%ah, %[err]\n"   // 失败：将 BIOS 错误码保存
        "2:\n"
        : [err] "=r"(err)
        : "a"(seg), "b"(off), "c"(cylsec), "d"(headdisk)
        : "cc"
    );
    return (uint16_t)err;
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

    size_t rootentry_pos = reserved_sector_count + (fat_table_count * fat_table_sector_count);
    size_t rootentry_length = root_entry_count * sizeof(rootentry);
    size_t entry_track = rootentry_pos / sectors_per_track;
    size_t entry_sector = rootentry_pos % sectors_per_track + 1;
    size_t entry_cylinder = entry_track / 2;
    size_t entry_head = entry_track % 2;

    size_t fat_table_pos = reserved_sector_count;
    size_t fat_table_track = fat_table_pos / 18;
    size_t fat_table_cylinder = fat_table_track / 2;
    size_t fat_table_head = fat_table_track % 2;
    size_t fat_table_sector = fat_table_pos % 18 + 1;

    ret = read_sector(fat_table_ptr, disknum, fat_table_cylinder, fat_table_head, fat_table_sector);
    if (ret) {
        puts("Failed to read FAT table sector!\n");
        return ret;
    }
    
    ret = read_sector(root_entry_ptr, disknum, entry_cylinder, entry_head, entry_sector);
    if (ret) {
        puts("Failed to read root entry sector!\n");
        return ret;
    }

    return ret;
}

bool read_fat_cls(far_ptr dest, size_t disknum, size_t first_cluster) {

    size_t first_data_sector = fat_header.BPB_RsvdSecCnt + (fat_header.BPB_NumFATs * fat_header.BPB_FATSz16) + fat_header.BPB_RootEntCnt * sizeof(rootentry) / 512;

    size_t cur_cls = first_cluster;
    size_t next_cls;
    for(size_t i = 0; i < 128; i++) { // max 64kb of file
        far_ptr cur_dest = {
            .segment = dest.segment,
            .offset = dest.offset + i * 512
        };
        // Convert cluster number to sector number (CHS)
        printf("Reading cluster 0x%x\n", cur_cls);
        uint16_t cur_sector = first_data_sector + (cur_cls - 2) * fat_header.BPB_SecPerClus;
        uint16_t track = cur_sector / 18;
        uint16_t cylinder = track / 2;
        uint16_t head = track % 2;
        uint16_t sector = cur_sector % 18 + 1;
        bool ret = read_sector(cur_dest, disknum, cylinder, head, sector);
        if(ret) return ret;

        // next_cls = heap_read16(cur_cls * 3 / 2);
        next_cls = far_ptr_read((far_ptr){
            .segment = fat_table_ptr.segment,
            .offset = fat_table_ptr.offset + (cur_cls * 3 / 2)
        });
        if (cur_cls % 2 == 0) {
            next_cls = next_cls & 0x0FFF; // Even cluster number
        } else {
            next_cls = (next_cls >> 4) & 0x0FFF; // Odd cluster number
        }
        if (next_cls == 0xFFF) {
            return false;
        }
        cur_cls = next_cls;
    }
    return false;
}