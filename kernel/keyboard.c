#include "keyboard.h"
#include "display.h"

// Block until user presses a key and return the keycode.
uint8_t poll_keyboard() {
    uint8_t keycode;
    asm volatile (
        "xor %%ah, %%ah\n"
        "int $0x16\n" // BIOS keyboard interrupt
        "mov %%al, %0\n" 
        : "=r"(keycode)
        :
        : "%ax"
    );
    return keycode;
}

// Get ascii code of the key press, if enter is pressed 
// return \n
// Terminal echo is on
char getchar() {
    uint8_t keycode = poll_keyboard();
    uint8_t ascii = keycode & 0x7F;
    switch (ascii) {
        case 0x08:
            ascii = '\b';
        case 0x09:
            ascii = '\t';
        case 0x0A:
        case 0x0D:
            ascii = '\n';
    }
    putchar(ascii);
    return ascii;
}

// Get a string with a maximum length of n, 
// stops if enter is pressed
char* gets(char *buf, size_t n) {
    for (size_t i = 0; i < n - 1; i++) {
        char c = getchar();
        if (c == '\n') {
            buf[i] = '\0';
            return buf;
        }
        buf[i] = c;
    }
    buf[n - 1] = '\0';
    return buf;
}


bool isalphanum(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isnum(char c) {
    return c >= '0' && c <= '9';
}

bool isalpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int getint() {
    bool ispositive = true;
    int ret = 0;
    char c = getchar();
    if (c == '-') ispositive = false;
    else if (isnum(c)) ret = c - '0';
    else return 0;

    while(1) {
        c = getchar();
        if (isnum(c)) {
            ret = 10 * ret + (c - '0');
        }
        else {
            if (!ispositive) ret *= -1;
            return ret;
        }
    }
}