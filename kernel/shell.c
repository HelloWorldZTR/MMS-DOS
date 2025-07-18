#include "type.h"
#include "display.h"
#include "keyboard.h"
#include "lib.h"
#include "disk.h"
#include "shell.h"

#define MAX_CWD_DEPTH 16
char cwd[MAX_CWD_DEPTH][11];
int cwd_index = 0;
char drv = 'A';
int disknum = 0;

Command commands[] = {
    {"ls", ls},
    {"cd", cd}
};
#define COMMAND_COUNT  (sizeof(commands) / sizeof(Command))

void halt() {
    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}

void ls(char *args) {
    rootentry entry;
    puts("\nFile Name\t\tType\tSize\n");
    puts("-------------------------------\n");
    for(size_t i = 0; i < fat_header.BPB_RootEntCnt; i++) {
        far_ptr entry_ptr = root_entry_ptr;
        entry_ptr.offset += i * sizeof(rootentry);
        heap2memcpy((uint8_t*)&entry, entry_ptr, sizeof(rootentry));

        if(entry.DIR_Name[0] == 0x00) {
            break; // End of entries
        }
        // Print the entry
        printf("- ");
        for(size_t j=0; j<11; j++) {
            putchar(entry.DIR_Name[j]);
        }
        if (entry.DIR_Attr & 0x10) {
            printf("\t<DIR>");
        }
        else {
            printf("\t\t\t%dB", entry.DIR_FileSize);
        }
        // printf("\t\t\t0x%x", entry.DIR_FstClus); // Debugß
        putchar('\n');
    }
}

void cd(char *name) {
    char formated_name[11];
    for (size_t i = 0; i < 11; i++) {
        formated_name[i] = ' ';
    }
    for (size_t i = 0; i < strlen(name); i++) {
        if (i < 11) {
            formated_name[i] = name[i];
        } else {
            puts("Directory name too long!\n");
            return; // Name too long
        }
    }

    rootentry entry;
    for(size_t i = 0; i < fat_header.BPB_RootEntCnt; i++) {
        far_ptr entry_ptr = root_entry_ptr;
        entry_ptr.offset += i * sizeof(rootentry);
        heap2memcpy((uint8_t*)&entry, entry_ptr, sizeof(rootentry));

        if(entry.DIR_Name[0] == '\0') {
            break; // End of entries
        }
        
        bool match = true;
        for (size_t j = 0; j < 11; j++) {
            if (entry.DIR_Name[j] != formated_name[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            if (entry.DIR_Attr & 0x10) { // Is directory
                if (entry.DIR_FstClus == 0x0) { // Is root directory
                    read_root_entry(disknum);
                    cwd_index = 0;
                    return;
                }
                if (strcmp(name, "..") == 0) { // Is upper directory
                    bool ret = read_fat_cls(root_entry_ptr, disknum, entry.DIR_FstClus);
                    if (!ret) { // Change directory successful
                        cwd_index--;
                    }
                    else {
                        puts("Error reading disk!\n");
                    }
                    return;
                }
                else if (strcmp(name, ".") == 0) { // Is current directory
                    return; // Do nothing
                }
                else if (cwd_index < MAX_CWD_DEPTH - 1) { // Is normal directory
                    bool ret = read_fat_cls(root_entry_ptr, disknum, entry.DIR_FstClus);
                    if (!ret) { // Change directory successful
                        memcpy(cwd[cwd_index], name, 11);
                        cwd_index++;
                        return; 
                    }
                    else{
                        puts("Error reading disk!\n");
                        return;
                    }
                } else {
                    puts("Too many directories!\n");
                    return;
                }
            } else {
                puts("Not a directory!\n");
                return;
            }
        }
    }
    printf("Directory %s not found!\n", name);
}

void print_prompt() {
    printf("%c:/", drv);
    for (int i = 0; i < cwd_index; i++) {
        printf("%s/", cwd[i]);
    }
    putchar('>');
}


void shell_main() {
    while(1) {
        print_prompt();
        char input[256];
        gets(input, 256);
        
        bool command_found = false;
        for (size_t i = 0; i < COMMAND_COUNT; i++) {
            if (start_with(input, commands[i].name)) {
                commands[i].func(input + strlen(commands[i].name) + 1);
                command_found = true;
                break;
            }
        }
        if (!command_found) {
            puts("Unknown command!\n");
        }
    }
}