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

    /* data+bss sit right after rodata and bss is emitted as zeros, so the
       flat binary is the whole image the kernel loads at 0x1000000 */
    .data : ALIGN(4K)
    {
        *(.data .data.*)
        *(.bss .bss.* COMMON)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.gnu*)
    }
}
