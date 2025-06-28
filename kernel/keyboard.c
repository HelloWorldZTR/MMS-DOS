#include "keyboard.h"

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