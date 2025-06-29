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
    asm volatile (
        "movw $0xB800, %%ax\n"
        "movw %%ax, %%es\n"
        "movw %[offset], %%di\n"
        "movw %[val], %%ax\n"
        "movw %%ax, %%es:(%%di)\n"
        :
        : [val]"r"(value), [offset]"r"(pos * 2)
        : "ax", "di", "es"
    );
}

static char read_from_buf(unsigned int pos) {
    uint16_t value;
    uint16_t offset = pos * 2;
    asm volatile (
        "movw $0xB800, %%ax\n\t"
        "movw %%ax, %%es\n\t"
        "movw %[offset], %%di\n\t"
        "movw %%es:(%%di), %%ax\n\t"
        "movw %%ax, %[val]\n\t"
        : [val] "=rm"(value)
        : [offset] "r"(offset)
        : "ax", "di", "es"
    );
    return (char)(value & 0xFF);
}

void test() {
    write_to_buf(0, 'M', fg, bg);
    char c = read_from_buf(0);
    if (c != 'M') {
        printf("Test failed: expected 'M' at 0x0, got '%c'\n", c);
    } else {
        printf("Video Memory Test passed!\n");
    }
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

// Set foreground and background color
void setcolor(Color foreground, Color background) {
    fg = foreground;
    bg = background;
}

// Clear the screen buffer to bg color
void clear() {
    for (int i = 0; i < ROWNUM * COLNUM; i++) {
        // video_memory[i] = translate(' ', bg, bg);
        write_to_buf(i, ' ', fg, bg);
    }
    row = 0;
    col = 0;
}

// Print a character to screen buffer
void putchar(const char c) {
    if (c == '\t') {
        col += 4; // Row increment will be taken care of in the next function call
        return;
    }
    if (c == '\n') {
        newline();
        return;
    } 
    else if (col >= COLNUM) {
        newline();
    }
    // video_memory[row*COLNUM + col] = translate(c, fg, bg);
    write_to_buf(row * COLNUM + col, c, fg, bg);
    col++; // Row increment will be taken care of in the next function call
}

// Print a char* to screen buffer
void puts(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}

//Print a rainbow colored char*
void puts_fancy(const char *str) {
    Color bak = fg;
    for (int i = 0; str[i] != '\0'; i++) {
        Color t = i % 14 + 1; //1-14 
        setcolor(t, bg);
        putchar(str[i]);
    }
    setcolor(bak, bg);
}

// Simple printf that supports %s, %c, and %d %x
void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while(*format) {
        if (*format == '%') {
            if(*(format+1) == 's') { // %s for string
                const char *str = va_arg(args, const char*);
                puts(str);
            }
            else if (*(format+1) == 'c') { // %c for character
                const char c = va_arg(args, char);
                putchar(c);
            }
            else if(*(format+1) == 'd') { // %d for decimal
                char buffer[12];
                int num = va_arg(args, int);
                puts(itoa(num, buffer, 10));
            }
            else if(*(format+1) == 'x') { // %x for hexadecimal
                char buffer[12];
                int num = va_arg(args, int);
                puts(itoa(num, buffer, 16));
            }
            else if (*(format+1) == '%') { // %%
                putchar('%');
            }
            else { // Unknown format specifier, just print it as is
                putchar('%');
            }
            format ++;
            if(! *format) return;
            format ++;
        }
        else {
            putchar(*format);
            format ++;
        }
    }
}

char* itoa(int num, char *buf, int base) {
    if(base < 2 || base > 36) {
        *buf = '\0'; // Invalid base
        return buf;
    }
    char conv[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *ptr = buf;
    bool is_negative = num < 0;
 
    if (is_negative && base == 10) {
        *ptr++ = '-';
        num = -num; // Make num positive for conversion
    }
    while(num) {
        *ptr++ = conv[num % base];
        num /= base;
    }
    *ptr = '\0';

    // Reverse the string
    char *start = buf + is_negative;
    char *end = ptr - 1;
    for(;start < end; start++, end--) {
        char temp = *start;
        *start = *end;
        *end = temp;
    }

    return buf;
}