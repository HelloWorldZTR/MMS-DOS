# MMS-DOS
Mimic MS-DOS

## Build

```bash
docker build -t dosdev .
docker run -it --rm -v ${PWD}:/src dosdev
make all
```

## Run

Build and run this on windows using qemu (with logs)
```bash
docker run -it --rm -v ${PWD}:/src -w /src dosdev make all 
'c:\Program Files\qemu\qemu-system-i386.exe' -fda .\system.img -d in_asm -D qemu.log
```