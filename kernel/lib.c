#include "lib.h"

void memcpy(char *dest, const char *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

void memset(char *dest, char value, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = value;
    }
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}