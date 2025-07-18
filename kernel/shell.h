#pragma once

#include "type.h"

void shell_main();
void halt();
void ls(char *args);
void cd(char *name);

typedef struct _command {
    char name[5]; // we don't expect commands longer than 4 characters
    void (*func)(char *args);
} Command;

// extern Command commands[];
// extern int COMMAND_COUNT;
extern char cwd[16][11];
extern int cwd_index;
extern char drv;
extern int disknum;