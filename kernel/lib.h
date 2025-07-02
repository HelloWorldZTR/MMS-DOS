#pragma once

#include "type.h"

void memcpy(near_ptr dest, const near_ptr src, size_t n);
void memset(near_ptr dest, char value, size_t n);

size_t strlen(const near_ptr str);
int strcmp(const char* str1, const char* str2);

// Far pointer functions
uint16_t far_ptr_read(far_ptr p);
void far_ptr_write(far_ptr p, uint16_t value);

// Heap management functions
far_ptr heap_pointer(uint16_t offset);
uint16_t heap_read16(uint16_t offset);
uint8_t heap_read8(uint16_t offset);
void heap_write16(uint16_t offset, uint16_t val);
void heap_write8(uint16_t offset, uint8_t value);
void heap2memcpy(near_ptr dest, far_ptr src, size_t n);
void mem2heapcpy(far_ptr dest, near_ptr src, size_t n);