# MMS-DOS
Mimic MS-DOS

## Build

```bash
docker build -t dosdev .
docker run --privileged -it --rm -v ${PWD}:/src dosdev
make all
```

## Run

Build and run this on windows using qemu (with logs)
```bash
docker run -it --rm -v ${PWD}:/src -w /src dosdev make all 
'c:\Program Files\qemu\qemu-system-i386.exe' -fda .\system.img -d in_asm -D qemu.log
```

Build and run this on mac/linux using qemu (with logs)
```bash
docker run -it --rm -v ${PWD}:/src -w /src dosdev make all 
qemu-system-i386 -fda .\system.img -d in_asm -D qemu.log
```

## Memory map
0x00000 - 0x0FFFF reserved
0x10000 - 0x1FFFF loader / FAT cache

0x20000 - 0x2FFFF kernel
...
0x90000 - 0x9FFFF heap
