# MMS-DOS
Mimic MS-DOS

## Build

```bash
docker build -t dosdev .
docker run -it --rm -v ${PWD}:/src dosdev
make all
```