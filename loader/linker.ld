OUTPUT_FORMAT("binary")
ENTRY(_start)
SECTIONS {
  . = 0x0;
 .text : {
    *(.text)
  }

  .rodata : {
    *(.rodata)
    *(.rodata*)
  }

  .data : {
    *(.data)
  }

  .bss : {
    *(.bss COMMON)
  }
}