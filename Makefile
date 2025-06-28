CFLAGS=-m16 -ffreestanding -fno-pic -fno-pie -nostdlib -fno-stack-protector -Wall
LDFLAGS=-m elf_i386 -T linker.ld -nostdlib

all: bootloader.bin kernel.bin system.img

bootloader.bin: bootloader/bootloader.asm
	nasm -o bootloader/bootloader.bin bootloader/bootloader.asm

kernel.bin: kernel/kernel.c kernel/type.h
	gcc $(CFLAGS) -c kernel/kernel.c -o kernel/kernel.o
	ld $(LDFLAGS) -o kernel/kernel.bin kernel/kernel.o

system.img: bootloader.bin kernel.bin
	dd if=/dev/zero of=system.img bs=512 count=2880 # Create a 1.44MB floppy disk image
	dd if=bootloader/bootloader.bin of=system.img conv=notrunc
	dd if=kernel/kernel.bin of=system.img bs=512 seek=1 conv=notrunc # write to sector 1

clean:
	rm -f bootloader/bootloader.bin kernel/kernel.bin kernel/kernel.o system.img

run: system.img
	qemu-system-i386 -fda system.img -display curses -monitor stdio
	# NOT RECOMMENDED!: use docker to run qemu is not a good idea
