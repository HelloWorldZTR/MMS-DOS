#pragma once

#include "type.h"

#pragma pack(push)
#pragma pack(1)
typedef struct _fat12header
{
	uint8_t BS_JmpBoot[3];
	uint8_t BS_OEMName[8];
	uint16_t BPB_BytePerSec;
	uint8_t BPB_SecPerClus;
	uint16_t BPB_RsvdSecCnt;
	uint8_t BPB_NumFATs;
	uint16_t BPB_RootEntCnt;
	uint16_t BPB_TotSec16;
	uint8_t BPB_Media;
	uint16_t BPB_FATSz16;
	uint16_t BPB_SecPerTrk;
	uint16_t BPB_NumHeads;
	uint32_t BPB_HiddSec;
	uint32_t BPB_TotSec32;
	uint8_t BS_DrvNum;
	uint8_t BS_Reserved1;
	uint8_t BS_Bootsig;
	uint8_t BS_VolID[4];
	uint8_t BS_VolLab[11];
	uint8_t BS_FileSysType[8];
	uint8_t BOOT_Code[448];
	uint8_t MBR_Flag[2];
}fat12header;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct _rootentry {
    uint8_t  DIR_Name[11];     // 文件名（8字节）+扩展名（3字节）
    uint8_t  DIR_Attr;         // 属性
    uint8_t  DIR_NTRes;        // 保留给Windows NT用（通常为0）
    uint8_t  DIR_CrtTimeTenth; // 创建时间的 10ms 单位
    uint16_t DIR_CrtTime;      // 创建时间
    uint16_t DIR_CrtDate;      // 创建日期
    uint16_t DIR_LstAccDate;   // 最后访问日期
    uint16_t DIR_FstClusHI;    // 起始簇号的高16位（FAT32才用，FAT12/16应为0）
    uint16_t DIR_WrtTime;      // 最后写入时间
    uint16_t DIR_WrtDate;      // 最后写入日期
    uint16_t DIR_FstClus;      // 起始簇号低16位（FAT12/16 实际使用这个）
    uint32_t DIR_FileSize;     // 文件大小（字节）
} rootentry;
#pragma pack(pop)

fat12header fat_header;
far_ptr file_buffer_ptr;
far_ptr root_entry_ptr;
far_ptr fat_table_ptr;

bool read_sector(far_ptr dest, size_t disknum, size_t cylindernum,  size_t headnum, size_t sectornum);
bool read_fat_header(size_t disknum);
bool load_fat_table(size_t disknum);
bool read_root_entry(size_t disknum);
bool read_fat_cls(far_ptr dest, size_t disknum, size_t first_cluster);

void fat2human(char* dest, const char* src);
bool human2fat(char* dest, const char* src);