CC=ia16-elf-gcc
LD=ia16-elf-ld
AS=ia16-elf-as

CFLAGS=-O0 -ffreestanding -fno-pic -Wall -Wextra -nostdlib
LDFLAGS=-m elf_i386_msdos_mz -nostdlib
LOADER_C_FILES = $(shell find loader -name '*.c')
LOADER_OBJ_FILES = $(LOADER_C_FILES:.c=.o)

KERNEL_C_FILES = $(shell find kernel -name '*.c')
KERNEL_OBJ_FILES = $(KERNEL_C_FILES:.c=.o)


all: system.img

#############################################
# Build the bootloader 
#############################################

# Assemble all assembly files
bootloader.bin: bootloader/bootloader.asm
	nasm -o bootloader/bootloader.bin bootloader/bootloader.asm

#############################################
# Build the loader
#############################################

# Compile all C files
loader/%.o: loader/%.c
	$(CC) $(CFLAGS) -c $< -o $@

entry.o: loader/entry.S
	$(AS) -c loader/entry.S -o loader/entry.o

LOADER.SYS: $(LOADER_OBJ_FILES) entry.o
	$(LD) $(LDFLAGS) -T loader/linker.ld -o loader/LOADER.SYS loader/entry.o $(LOADER_OBJ_FILES)

#############################################
# Build the kernel
#############################################

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@
entry_kernel.o: kernel/entry_kernel.S
	$(AS) -c kernel/entry_kernel.S -o kernel/entry_kernel.o

KERNEL.SYS: $(KERNEL_OBJ_FILES) entry_kernel.o
	$(LD) $(LDFLAGS) -T kernel/linker.ld -o fs/KERNEL.SYS kernel/entry_kernel.o $(KERNEL_OBJ_FILES)

#############################################
# Piece together the system image
#############################################

system.img: bootloader.bin LOADER.SYS KERNEL.SYS
	dd if=/dev/zero of=system.img bs=512 count=2880
	mkfs.fat -F 12 -R 1 system.img
	dd if=bootloader/bootloader.bin of=system.img conv=notrunc
	mkdir -p /mnt/tmp
	mount -o loop system.img /mnt/tmp
	cp loader/LOADER.SYS /mnt/tmp/
	cp -R fs/* /mnt/tmp/
	umount /mnt/tmp
	rm -rf /mnt/tmp


#############################################
# Miscellaneous
#############################################
clean:
	rm -f bootloader/bootloader.bin
	rm -f loader/*.o
	rm -f loader/LOADER.SYS
	rm -f kernel/*.o
	rm -f fs/KERNEL.SYS
	rm -f system.img

run: system.img
	qemu-system-i386 -fda system.img -display curses -monitor stdio
	# NOT RECOMMENDED!: use docker to run qemu is not a good idea
