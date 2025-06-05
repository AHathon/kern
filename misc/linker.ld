ENTRY(_start)

SECTIONS
{
    . = 0x1000000;
    .text : { *(.text .text.*) }

    . = 0x8000000;
    .data : { *(.data .data.*) }
    .bss : { *(.bss .COMMON) }
}
