#pragma once
#include "type.h"

typedef enum {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHT_GRAY = 7,
    DARK_GRAY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
} Color;

#define ROWNUM 25
#define COLNUM 80

extern volatile uint16_t* video_memory;  // Video memory address
extern uint16_t row;            // Current row
extern uint16_t col;            // Current column
extern Color fg;                // Foreground color
extern Color bg;                // Background color

void setcolor(Color, Color);
void clear();
void putchar(const char);
void puts(const char *);
void puts_fancy(const char *);
void printf(const char *format, ...);
char* itoa(int num, char *str, int base);

