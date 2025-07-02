CC=ia16-elf-gcc
LD=ia16-elf-ld
AS=ia16-elf-as

CFLAGS=-O0 -ffreestanding -fno-pic -Wall -Wextra -nostdlib
LDFLAGS=-m elf_i386_msdos_mz -nostdlib
KERNEL_C_FILES = $(shell find kernel -name '*.c')
KERNEL_OBJ_FILES = $(KERNEL_C_FILES:.c=.o)
KERNEL_LINK_ORDER := kernel/kernel.o $(filter-out kernel/kernel.o, $(KERNEL_OBJ_FILES))  # main function first

SHELL_C_FILES = $(shell find shell -name '*.c')
SHELL_OBJ_FILES = $(SHELL_C_FILES:.c=.o)

all: system.img

#############################################
# Build the bootloader 
#############################################

# Assemble all assembly files
bootloader.bin: bootloader/bootloader.asm
	nasm -o bootloader/bootloader.bin bootloader/bootloader.asm

#############################################
# Build the kernel
#############################################

# Compile all C files
kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

entry.o: kernel/entry.S
	$(AS) -c kernel/entry.S -o kernel/entry.o

# Link the kernel object files
KERNEL.COM: $(KERNEL_OBJ_FILES) entry.o
	$(LD) $(LDFLAGS) -T kernel/linker.ld -o kernel/kernel.elf kernel/entry.o $(KERNEL_LINK_ORDER)
	objcopy -O binary kernel/kernel.elf kernel/KERNEL.COM

#############################################
# Build the shell.com
#############################################

shell/%.o: shell/%.c
	$(CC) $(CFLAGS) -c $< -o $@

SHELL.COM: $(SHELL_OBJ_FILES)
	$(LD) $(LDFLAGS) -T shell/linker.ld -o shell/shell.elf $(SHELL_OBJ_FILES)
	objcopy -O binary shell/shell.elf shell/SHELL.COM

#############################################
# Piece together the system image
#############################################

system.img: bootloader.bin KERNEL.COM
	dd if=/dev/zero of=system.img bs=512 count=2880
	mkfs.fat -F 12 -R 1 system.img
	dd if=bootloader/bootloader.bin of=system.img conv=notrunc
	mkdir -p /mnt/tmp
	mount -o loop system.img /mnt/tmp
	cp kernel/KERNEL.COM /mnt/tmp/
	cp -R fs/* /mnt/tmp/
	umount /mnt/tmp
	rm -rf /mnt/tmp


#############################################
# Miscellaneous
#############################################
clean:
	rm -f kernel/*.o kernel/KERNEL.COM bootloader/bootloader.bin system.img kernel/kernel.elf

run: system.img
	qemu-system-i386 -fda system.img -display curses -monitor stdio
	# NOT RECOMMENDED!: use docker to run qemu is not a good idea
CC=ia16-elf-gcc
LD=ia16-elf-ld
AS=ia16-elf-as

CFLAGS=-O0 -ffreestanding -fno-pic -Wall -Wextra -nostdlib
LDFLAGS=-m elf_i386_msdos_mz -nostdlib
KERNEL_C_FILES = $(shell find kernel -name '*.c')
KERNEL_OBJ_FILES = $(KERNEL_C_FILES:.c=.o)
KERNEL_LINK_ORDER := kernel/kernel.o $(filter-out kernel/kernel.o, $(KERNEL_OBJ_FILES))  # main function first

SHELL_C_FILES = $(shell find shell -name '*.c')
SHELL_OBJ_FILES = $(SHELL_C_FILES:.c=.o)

all: system.img

#############################################
# Build the bootloader 
#############################################

# Assemble all assembly files
bootloader.bin: bootloader/bootloader.asm
	nasm -o bootloader/bootloader.bin bootloader/bootloader.asm

#############################################
# Build the kernel
#############################################

# Compile all C files
kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

entry.o: kernel/entry.S
	$(AS) -c kernel/entry.S -o kernel/entry.o

# Link the kernel object files
KERNEL.COM: $(KERNEL_OBJ_FILES) entry.o
	$(LD) $(LDFLAGS) -T kernel/linker.ld -o kernel/kernel.elf kernel/entry.o $(KERNEL_LINK_ORDER)

#############################################
# Build the shell.com
#############################################

shell/%.o: shell/%.c
	$(CC) $(CFLAGS) -c $< -o $@

SHELL.COM: $(SHELL_OBJ_FILES)
	$(LD) $(LDFLAGS) -T shell/linker.ld -o shell/shell.elf $(SHELL_OBJ_FILES)
	objcopy -O binary shell/shell.elf shell/SHELL.COM

#############################################
# Piece together the system image
#############################################

system.img: bootloader.bin KERNEL.COM
	dd if=/dev/zero of=system.img bs=512 count=2880
	mkfs.fat -F 12 -R 1 system.img
	dd if=bootloader/bootloader.bin of=system.img conv=notrunc
	mkdir -p /mnt/tmp
	mount -o loop system.img /mnt/tmp
	cp kernel/KERNEL.COM /mnt/tmp/
	cp -R fs/* /mnt/tmp/
	umount /mnt/tmp
	rm -rf /mnt/tmp


#############################################
# Miscellaneous
#############################################
clean:
	rm -f kernel/*.o kernel/KERNEL.COM bootloader/bootloader.bin system.img kernel/kernel.elf

run: system.img
	qemu-system-i386 -fda system.img -display curses -monitor stdio
	# NOT RECOMMENDED!: use docker to run qemu is not a good idea
