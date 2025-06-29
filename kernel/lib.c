#include "lib.h"
#include "type.h"

#define HEAP_SEGMENT_ADDRESS 0x9000

void memcpy(char *dest, const char *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

void memset(char *dest, char value, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = value;
    }
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

uint16_t far_pointer_read(far_pointer p) {
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
        : [ret] "=rm"(ret)
        : [seg] "r"(seg), [off] "r"(off)
        : "ax", "di", "ds"
    );
    return ret;
}

void far_pointer_write(far_pointer p, uint16_t value) {
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

far_pointer heap_pointer(uint16_t offset) {
    far_pointer p;
    p.segment = HEAP_SEGMENT_ADDRESS;
    p.offset = offset;
    return p;
}

uint16_t heap_read(uint16_t offset) {
    far_pointer p = heap_pointer(offset);
    return far_pointer_read(p);
}

void heap_write(uint16_t offset, uint16_t value) {
    far_pointer p = heap_pointer(offset);
    far_pointer_write(p, value);
}
