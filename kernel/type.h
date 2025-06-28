#pragma once

// Basic types
#define NULL ((void*)0)
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
#define size_t unsigned int
#define bool int
#define true 1
#define false 0

// Variable argument macros
#define va_list char*
#define va_start(ap, param) (ap = (char*)&param + sizeof(param))
#define va_arg(ap, type) (* ((type*)((ap += sizeof(type)) - sizeof(type))))
#define va_end(ap) (ap = NULL)

