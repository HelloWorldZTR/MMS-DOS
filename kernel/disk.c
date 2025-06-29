#include "disk.h"

bool read_sector(far_pointer dest, size_t disknum, size_t tracknum, size_t sectornum, size_t headnum, size_t n) {
    uint16_t seg = dest.segment;
    uint16_t off = dest.offset;
    bool fail = false;
    uint16_t tracsec = tracknum << 8 | sectornum;
    uint16_t headdisk = headnum << 8 | disknum;
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
        : "a"(seg), "b"(off), "c"(tracsec), "d"(headdisk)
        :
    );
    return fail;
}