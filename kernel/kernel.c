/* kernel.c */
#include "type.h"

/* Entry point for the kernel */
void kernel_main() {
    uint16_t *video_memory = (uint16_t *)0xB8000; // Video memory address
    video_memory[0] = 'H' | 0x0F00; // 'H' character with white foreground
    video_memory[1] = 'i' | 0x0F00; // 'i' character with white foreground
    while(1){
        // Freeze the kernel to prevent it from exiting
    }
}