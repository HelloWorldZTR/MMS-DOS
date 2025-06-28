#include "type.h"
#include "display.h"

uint16_t row = 0;
uint16_t col = 0;
Color fg = WHITE;
Color bg = BLACK;


uint16_t translate(const char c, Color fg, Color bg) {
    return (uint16_t)c | ((uint16_t)fg << 8) | ((uint16_t)bg << 12);
}

static void write_to_buf(unsigned int pos, char c, Color fg, Color bg) {
    uint16_t value = translate(c, fg, bg);
    __asm__ volatile (
        "movw $0xB800, %%ax\n"
        "movw %%ax, %%es\n"
        "movw %1, %%di\n"
        "movw %0, %%ax\n"
        "movw %%ax, %%es:(%%di)\n"
        :
        : "r"(value), "r"(pos * 2)
        : "ax", "di", "es"
    );
}
static char read_from_buf(unsigned int pos) {
    uint16_t value;
    __asm__ volatile (
        "movw $0xB800, %%ax\n"
        "movw %%ax, %%es\n"
        "movw %1, %%di\n"
        "movw %%es:(%%di), %%ax\n"
        : "=a"(value)
        : "r"(pos * 2)
        : "di", "es"
    );
    return (char)(value & 0xFF);
}

void newline() {
    if (row + 1 == ROWNUM) {
        for (int i = 1; i < ROWNUM; i++) {
            for (int j = 0; j < COLNUM; j++) {
                //video_memory[(i-1)*COLNUM + j] = video_memory[i*COLNUM + j];
                write_to_buf((i-1)*COLNUM + j, read_from_buf(i*COLNUM + j), fg, bg);
            }
        }
        col = 0;
        row = ROWNUM - 1;
    }
    else {
        row ++;
        col = 0;
    }
}


void setcolor(Color foreground, Color background) {
    fg = foreground;
    bg = background;
}

void clear() {
    for (int i = 0; i < ROWNUM * COLNUM; i++) {
        // video_memory[i] = translate(' ', bg, bg);
        write_to_buf(i, ' ', fg, bg);
    }
    row = 0;
    col = 0;
}

void putchar(const char c) {
    if (c == '\n') {
        newline();
        return;
    } 
    else if (col == COLNUM) {
        newline();
    }
    // video_memory[row*COLNUM + col] = translate(c, fg, bg);
    write_to_buf(row * COLNUM + col, c, fg, bg);
    col++; // Row increment will be taken care of in the next function call
}


void puts(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}
