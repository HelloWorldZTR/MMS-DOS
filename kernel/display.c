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

// Show a blinking cursor at row, col
void move_cursor(uint16_t row, uint16_t col) {
    uint16_t rowcol = row << 8 | col;
    asm volatile(
        "movb $0x02, %%ah\n"
        "movw %[rowcol], %%dx\n"
        "movb $0x00, %%bh\n"
        "int $0x10\n"
        :
        : [rowcol] "r"(rowcol)
        : "ax", "dx"
    );
}

// Back space a character
// Go to the previous line if cursor is already at the beginning of the line
void backspace() {
    if (col == 0) {
        if (row > 0) {
            row --;
            col = COLNUM - 1;
            write_to_buf(row * COLNUM + col, ' ', fg, bg);
            move_cursor(row, col);
        }
    }
    else {
        col --;
        write_to_buf(row * COLNUM + col, ' ', fg, bg);
        move_cursor(row, col);
    }
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

// Move the cursor to the next line
// Scroll the screen if necessary
void newline() {
    if (row + 1 == ROWNUM) {
        // Scroll the screen up
        for (int i = 1; i < ROWNUM; i++) {
            for (int j = 0; j < COLNUM; j++) {
                //video_memory[(i-1)*COLNUM + j] = video_memory[i*COLNUM + j];
                write_to_buf((i-1)*COLNUM + j, read_from_buf(i*COLNUM + j), fg, bg);
            }
        }
        // Clear the last row
        for (int j = 0; j < COLNUM; j++) {
            write_to_buf((ROWNUM-1)*COLNUM + j, ' ', fg, bg);
        }
        col = 0;
        row = ROWNUM - 1;
    }
    else {
        // Move the cursor down
        row ++;
        col = 0;
    }
    move_cursor(row, col);
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
    move_cursor(row, col);
}

// Print a character to screen buffer
void putchar(const char c) {
    if (c == '\t') {
        col += 4;
        if (col >= COLNUM) {
            newline(); // This handles row and col change
        }
        return;
    }
    else if (c == '\n') {
        newline();
        return;
    } 
    else if (c == '\b') {
        backspace();
        return;
    }   
    else {
        write_to_buf(row * COLNUM + col, c, fg, bg);
        col++;
        if (col >= COLNUM) {
            newline(); // This handles row and col change
        } else {
            move_cursor(row, col);
        }
    }
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
    bool is_negative = num < 0 && base == 10;
 
    if (is_negative && base == 10) {
        *ptr++ = '-';
        num = -num; // Make num positive for conversion
    }

    if (num == 0) {
        *ptr = '0'; // Handle zero case
        ptr++;
        *ptr = '\0';
        return buf;
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