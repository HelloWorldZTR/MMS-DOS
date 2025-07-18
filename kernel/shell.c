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

void halt() {
    puts("System halted.\n");
    asm volatile (
        "cli\n" // Disable interrupts
        "hlt\n" // Halt the CPU
    );
}

void ls(char *args) {
    rootentry entry;
    char formated_name[13];
    puts("\nFile Name\t\tType\tSize\n");
    puts("-------------------------------\n");
    for(size_t i = 0; i < fat_header.BPB_RootEntCnt; i++) {
        far_ptr entry_ptr = root_entry_ptr;
        entry_ptr.offset += i * sizeof(rootentry);
        heap2memcpy((uint8_t*)&entry, entry_ptr, sizeof(rootentry));
        fat2human(formated_name, (const char*)entry.DIR_Name);

        if(entry.DIR_Name[0] == 0x00) {
            break; // End of entries
        }
        // Print the entry
        printf("- ");
        puts(formated_name);
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
    if(name == NULL || name[0] == '\0') {
        puts("No directory name provided!\n");
        return;
    }
    char formated_name[11];
    int ret = human2fat(formated_name, name);
    if (ret) {
        puts("Invalid directory name!\n");
        return;
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
                    int ret = read_fat_file(root_entry_ptr, disknum, entry.DIR_FstClus);
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
                    int ret = read_fat_file(root_entry_ptr, disknum, entry.DIR_FstClus);
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

void cls(char* args) {
    clear();
}

void pwd(char* args) {
    printf("%c:/", drv);
    for (int i = 0; i < cwd_index; i++) {
        printf("%s/", cwd[i]);
    }
    putchar('\n');
}

void cat(char* name) {
    if (name == NULL || name[0] == '\0') {
        puts("No file name provided!\n");
        return;
    }
    char formated_name[11];
    int ret = human2fat(formated_name, name);
    if (ret) {
        puts("Invalid file name!\n");
        return;
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
            if (!(entry.DIR_Attr & 0x10)) { // Is not a directory
                // Read the file content to buffer
                read_fat_file(file_buffer_ptr, disknum, entry.DIR_FstClus);
                // Print the file content
                size_t file_size = entry.DIR_FileSize;
                char temp_buffer[512];
                far_ptr temp_ptr = file_buffer_ptr;
                while(file_size) {
                    size_t to_read = file_size < 512 ? file_size : 512;
                    heap2memcpy(temp_buffer, temp_ptr, to_read);
                    for (size_t j = 0; j < to_read; j++) {
                        putchar(temp_buffer[j]);
                    }
                    file_size -= to_read;
                    temp_ptr.offset += to_read;
                }
                putchar('\n');
                return;
            } else {
                puts("Not a file!\n");
                return;
            }
        }
    }
    printf("File %s not found!\n", name);
}


void test_fn(char* args) {
    puts("Test???\n");
    char test_str[] = "Hello-Wo-rld!";
    char* token = strtok(test_str, "-");
    while (token) {
        printf("Token: %s\n", token);
        token = strtok(NULL, "-");
    }
}

void print_prompt() {
    printf("%c:/", drv);
    for (int i = 0; i < cwd_index; i++) {
        printf("%s/", cwd[i]);
    }
    putchar('>');
}

void echo(char* args) {
    if (args) puts(args);
    putchar('\n');
}

Command commands[] = {
    {"ls", ls},
    {"cd", cd},
    {"cls", cls},
    {"pwd", pwd},
    {"test", test_fn},
    {"cat", cat},
    {"echo", echo},
    {"exit", halt}
};
#define COMMAND_COUNT  (sizeof(commands) / sizeof(Command))


void shell_main() {
    while(1) {
        print_prompt();
        char input[256];
        gets(input, 256);
        
        bool command_found = false;
        char* command_name = strtok(input, " ");
        char* args = strtok(NULL, " "); // Could be NULL
        for (size_t i = 0; i < COMMAND_COUNT; i++) {
            if (strcmp(command_name, commands[i].name) == 0) {
                commands[i].func(args);
                command_found = true;
                break;
            }
        }
        if (!command_found) {
            puts("Unknown command!\n");
        }
    }
}