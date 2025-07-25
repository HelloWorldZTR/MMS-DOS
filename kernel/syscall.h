/****************************************************************
 * @file syscall.h
 * @brief System call definitions and interrupt handler installation.
 * @note: Store the values of return value registers before calling
 *       `int 0x21`.
 * -----------------------------------------------------------------
 * @author Andy Zhang
 * @date 2025-07
 * @copyright MIT
 *******************************************************************/
#pragma once

#include "type.h"

// System call numbers
#define SYS_PRINT_CHAR     0x02  // AL = character
#define SYS_PRINT_STRING   0x09  // DS:DX = '\0'-terminated string
#define SYS_READ_CHAR      0x01  // AL = character
#define SYS_READ_STRING    0x0A  // DS:DX = buffer (first byte = max len)
#define SYS_EXIT           0x4C  // AL = exit code

// TODO: Implement file io syscalls
// #define SYS_OPEN_FILE      0x3D  // DS:DX = filename, AL = access mode
// #define SYS_CLOSE_FILE     0x3E  // BX = file handle
// #define SYS_READ_FILE      0x3F  // BX = file handle, CX = bytes, DS:DX = buffer
// #define SYS_WRITE_FILE     0x40  // BX = file handle, CX = bytes, DS:DX = buffer
// #define SYS_CREATE_FILE    0x3C  // DS:DX = filename, CX = attributes
// #define SYS_DELETE_FILE    0x41  // DS:DX = filename

#define SYS_GET_TIME       0x2C  // CH = hour, CL = minute, DH = second, DL = 1/100s
#define SYS_GET_DATE       0x2A  // CX = year, DH = month, DL = day

#define SYS_ALLOC_MEM      0x48  // BX = paragraphs, returns AX = segment
#define SYS_FREE_MEM       0x49  // ES = segment
#define SYS_RESIZE_MEM     0x4A  // ES = segment, BX = new size

#define SYS_EXEC           0x4B  // DS:DX = filename, other regs vary

typedef enum _register {
    REG_AX = 0,
    REG_BX,
    REG_CX,
    REG_DX,
    REG_SI,
    REG_DI,
    REG_BP,
    REG_DS,
    REG_ES,
} Register;

void install_interrupt_handlers();