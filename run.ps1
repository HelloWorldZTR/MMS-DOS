docker run -it --rm --privileged -v ${PWD}:/src dosdev make all
& 'c:\Program Files\qemu\qemu-system-i386.exe' -fda .\system.img -d in_asm -D qemu.log