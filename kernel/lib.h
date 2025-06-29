#pragma once

#include "type.h"

void memcpy(char *dest, const char *src, size_t n);
void memset(char *dest, char value, size_t n);
size_t strlen(const char *str);
uint16_t far_pointer_read(far_pointer p);
void far_pointer_write(far_pointer p, uint16_t value);

far_pointer heap_pointer(uint16_t offset);
uint16_t heap_read(uint16_t offset);
void heap_write(uint16_t offset, uint16_t val);
