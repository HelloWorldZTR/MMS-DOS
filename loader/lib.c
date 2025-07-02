#include "lib.h"
#include "type.h"

#define HEAP_SEGMENT_ADDRESS 0x9000

void memcpy(near_ptr dest, const near_ptr src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

size_t strlen(const near_ptr str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

uint16_t far_ptr_read(far_ptr p) {
    uint16_t ret;
    uint16_t seg = p.segment;
    uint16_t off = p.offset;
    asm volatile (
        "push %%ds\n"
        "movw %[seg], %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %[off], %%di\n"
        "movw %%ds:(%%di), %%ax\n"
        "movw %%ax, %[ret]\n"
        "pop %%ds\n"
        : [ret] "=r"(ret)
        : [seg] "r"(seg), [off] "r"(off)
        : "ax", "di", "ds"
    );
    return ret;
}

void far_ptr_write(far_ptr p, uint16_t value) {
    uint16_t seg = p.segment;
    uint16_t off = p.offset;
    asm volatile (
        "push %%ds\n"
        "movw %[seg], %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %[off], %%di\n"
        "movw %[val], %%bx\n"
        "movw %%bx, %%ds:(%%di)\n"
        "pop %%ds\n"
        :
        : [seg] "r"(seg), [off] "r"(off), [val] "r"(value)
        : "ax", "di"
    );
}

far_ptr heap_pointer(uint16_t offset) {
    far_ptr p;
    p.segment = HEAP_SEGMENT_ADDRESS;
    p.offset = offset;
    return p;
}

uint16_t heap_read16(uint16_t offset) {
    far_ptr p = heap_pointer(offset);
    return far_ptr_read(p);
}


void heap2memcpy(near_ptr dest, far_ptr src, size_t n) {

    for (size_t i = 0; i < n; i++) {
        dest[i] = far_ptr_read(src);
        src.offset++;
    }
}
