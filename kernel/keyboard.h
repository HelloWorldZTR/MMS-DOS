#pragma once

#include "type.h"

uint8_t poll_keyboard();

int getint();
char getchar();
char* gets(char *buf, size_t n);
bool isalpha(char c);
bool isnum(char c);
bool isalphanum(char c);
int getint();