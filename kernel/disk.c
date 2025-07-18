/* disk.c */
#include "disk.h"
#include "lib.h"
#include "display.h"

fat12header fat_header;
far_ptr file_buffer_ptr = {// Max 8kb 0x12000 - 0x13FFF
    .segment = 0x1000,
    .offset = 0x2000
};
far_ptr root_entry_ptr = {// Max 4kb 0x11000 - 0x11FFF
    .segment = 0x1000,
    .offset = 0x1000
};
far_ptr fat_table_ptr = { // Max 4kb 0x10000 - 0x10FFF
    .segment = 0x1000,
    .offset = 0x0000
};

// Read one sector from the disk using BIOS interrupt 0x13
// dest: far pointer to the destination memory
// disknum: 0 by default
// cylinder, head, sector: CHS addressing
// return error code (0 for success, non-zero for failure)
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

// Read the FAT header from the disknum and store it in fat_header
// Returns true if the read was successful, false otherwise
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

// Load the FAT table and root entries into memory
// and store then into fat_table_ptr and root_entry_ptr
// fat_table_ptr: far pointer to the FAT table
// root_entry_ptr: far pointer to the root entries
// disknum: the disk number to read from
// Returns true if the read was successful, false otherwise
bool load_fat_table(size_t disknum) {
    bool ret = false;
    // Dont ask me why I copy all these variables
    // They will be optimized by the compiler anyway
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
    }

    return ret;
}

// Read the root entries from the disk and store them in root_entry_ptr
// disknum: the disk number to read from
// Returns true if the read was successful, false otherwise
bool read_root_entry(size_t disknum) {
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
    ret = read_sector(root_entry_ptr, disknum, entry_cylinder, entry_head, entry_sector);
    if (ret) {
        puts("Failed to read root entry sector!\n");
    }
    return ret;
}

// Read the FAT cluster chain starting from first_cluster
// dest: far pointer to the destination memory
// disknum: the disk number to read from
// first_cluster: the starting cluster number
// Returns true if the read was successful, false otherwise
// TODO: Add size constraint to prevent blowing up file buffer
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
        // printf("Reading cluster 0x%x\n", cur_cls);
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

        if (next_cls >= 0xFF0 && next_cls != 0xFF7) { // End of cluster chain and not a bad cluster
            return false;
        }
        cur_cls = next_cls;
    }
    return false;
}

// Format a FAT name from a source string to a destination string
// Out put is exactly 12 characters long
// dest: destination string (at least 13 bytes long)
// src: source string (11 bytes long)
// AAAAAAA.BBB
// AAAAAAABBB
void fat2human(char* dest, const char* src) {
    for(size_t i = 0; i < 12; i++) dest[i] = ' ';
    bool has_dot = false;
    if(src[8] != ' ') has_dot = true;

    size_t j = 0;
    for (size_t i = 0; i < 7; i++) { // Copy file name
        if (src[i] == ' ') break;
        dest[j++] = src[i];
    }
    if (has_dot) {
        dest[j++] = '.';
    }
    for (size_t i = 8; i < 11; i++) { // Copy file extension
        if (src[i] == ' ') break;
        dest[j++] = src[i];
    }
    dest[12] = '\0';
}

// Convert a human-readable file name to a FAT name
// dest: destination string exactly 11 bytes long
// src: source string filename < 7 chars, extension < 3 chars
// AAAAAAABBB
// AAAAAAA.BBB
// Returns false if the conversion was successful, true otherwise
bool human2fat(char* dest, const char* src) {
    // Special cases
    if (strcmp(src, ".") == 0) {
        memset(dest, ' ', 11);
        dest[0] = '.';
        return false;
    }
    else if (strcmp(src, "..") == 0) {
        memset(dest, ' ', 11);
        dest[0] = dest[1] = '.';
        return false;
    }
    // Check length
    char* dot = strchr(src, '.');
    if (dot) {
        size_t name_len = dot - src;
        if (name_len > 8) {
            puts("File name too long!\n");
            return true; // Name too long
        }
        if (strlen(src) - name_len - 1 > 3) {
            puts("File extension too long!\n");
            return true; // Extension too long
        }
    } 
    else {
        if (strlen(src) > 8) {
            puts("File name too long!\n");
            return true; // Name too long
        }
    }
    // Clear destination (Only after checking length)
    memset(dest, ' ', 11);
    // Copy name
    size_t name_len = dot ? dot - src : strlen(src);
    for(size_t i = 0; i < name_len; i++) {
        dest[i] = src[i];
    }
    // Copy extension
    if (dot) {
        size_t ext_len = strlen(src) - name_len - 1;
        for (size_t i = 0; i < ext_len; i++) {
            dest[8 + i] = dot[1 + i]; // dot[0] is '.'
        }
    }
    return false;
}