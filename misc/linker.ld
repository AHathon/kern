ENTRY(_start)

SECTIONS
{
    . = 0x1000000;
    .text : ALIGN(4K)
    {
        KEEP(*(.text.start))
        *(.text .text.*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata .rodata.*)
    }

    .eh_frame : ALIGN(8)
    {
        *(.eh_frame)
    }

    . = 0x8000000;
    .data : ALIGN(8)
    {
        *(.data .data.*)
    }

    .bss : ALIGN(8)
    {
        *(.bss .bss.* .COMMON)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.gnu*)
    }
}
