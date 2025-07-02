#include "disk.h"

bool read_sector(far_ptr dest, size_t disknum, size_t cylindernum, size_t headnum, size_t sectornum, size_t n) {
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