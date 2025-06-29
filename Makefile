CC=ia16-elf-gcc
LD=ia16-elf-ld
AS=ia16-elf-as

CFLAGS=-O0 -ffreestanding -fno-pic -Wall -Wextra -nostdlib
LDFLAGS=-m elf_i386_msdos_mz -T linker.ld -nostdlib
KERNEL_C_FILES = $(shell find kernel -name '*.c')
KERNEL_OBJ_FILES = $(KERNEL_C_FILES:.c=.o)
KERNEL_LINK_ORDER := kernel/kernel.o $(filter-out kernel/kernel.o, $(KERNEL_OBJ_FILES))  # main function first

all: system.img

# Compile all C files
kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble all assembly files
bootloader.bin: bootloader/bootloader.asm
	nasm -o bootloader/bootloader.bin bootloader/bootloader.asm

entry.o: kernel/entry.S
	$(AS) -c kernel/entry.S -o kernel/entry.o

# Link the kernel object files
kernel.bin: $(KERNEL_OBJ_FILES) entry.o
	$(LD) $(LDFLAGS) -o kernel/kernel.elf kernel/entry.o $(KERNEL_LINK_ORDER)
	objcopy -O binary kernel/kernel.elf kernel/kernel.bin

# Piece together the system image
system.img: bootloader.bin kernel.bin
	dd if=/dev/zero of=system.img bs=512 count=2880
	mkfs.fat -F 12 -R 9 system.img
	dd if=bootloader/bootloader.bin of=system.img conv=notrunc
	dd if=kernel/kernel.bin of=system.img bs=512 seek=1 conv=notrunc
	mkdir -p /mnt/tmp
	mount -o loop system.img /mnt/tmp
	cp -R fs/* /mnt/tmp/
	umount /mnt/tmp
	rm -rf /mnt/tmp

clean:
	rm -f kernel/*.o kernel/kernel.bin bootloader/bootloader.bin system.img kernel/kernel.elf

run: system.img
	qemu-system-i386 -fda system.img -display curses -monitor stdio
	# NOT RECOMMENDED!: use docker to run qemu is not a good idea
