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

typedef struct _rootentry
{
	uint8_t DIR_Name[11];
	uint8_t DIR_Attr;
	uint8_t reserve[10];
	uint16_t DIR_WrtTime;
	uint16_t DIR_WrtDate;
	uint16_t DIR_FstClus;
	uint32_t DIR_FileSize;
}rootentry;


#pragma pack(pop)

bool read_sector(far_pointer dest, size_t disknum, size_t tracknum, size_t sectornum, size_t headnum, size_t n);