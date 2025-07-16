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

// Get ascii code of the key press
// Return \0 if key press is not a printable character
// Terminal echo is on
char getchar() {
    uint8_t keycode = poll_keyboard();
    if (keycode < 0x20 || keycode == 0x7F) {
        return '\0'; // Non-printable character
    }
    else {
        uint8_t ascii = keycode & 0x7F;
        putchar(ascii);
        return ascii;
    }
}

// Get a string with a maximum length of n, 
// stops if enter is pressed
char* gets(char *buf, size_t n) {
    size_t i = 0;
    while(1) {
        uint8_t keycode = poll_keyboard();
        char c = keycode & 0x7F; // Get ascii code
        switch (c) {
            case 0x08: // Backspace
                c = '\b';
                if (i > 0) {
                    i--;
                    buf[i] = '\0';
                    putchar(c); // Echo backspace
                }
                break;
            case 0x09:
                c = '\t';
                for (size_t j = 0; j < 4; j++) { // Tab is 4 spaces
                    if (i < n - 1) {
                        buf[i++] = ' ';
                        putchar(' ');
                    }
                }
                break;
            case 0x0A:
            case 0x0D:
                c = '\n';
                buf[i] = '\0';
                putchar(c);
                return buf;
            default:
                buf[i++] = c;
                putchar(c);
                break;
        }

        if (i >= n - 1) {
            buf[n - 1] = '\0';
            return buf;
        }
    }
    return buf; // Should never reach here
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