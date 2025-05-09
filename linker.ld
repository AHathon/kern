ENTRY(_start)

SECTIONS
{
    . = 0x80000;
    .text :
    {
        __start = .;
        __text_start = .;
        KEEP(*(.text.boot))
        *(.text)
        . = ALIGN(4096);
        __text_end = .;
    }

    .rodata :
    {
        __rodata_start = .;
        *(.rodata)
        . = ALIGN(4096);
        __rodata_end = .;
    }
    
    .data :
    {
        __data_start = .;
        *(.data)
        . = ALIGN(4096);
        __data_end = .;
    }

    .bss :
    {
        __bss_start = .;
        *(.bss)
        . = ALIGN(4096);
        __bss_end = .;
    }

    __bss_size = __bss_end - __bss_start;
    __stack_bottom = .;
    . = . + 0x1000; /* 4KB stack for core 0 */
    __stack_top = .;
    . = ALIGN(4096);
    __page_table = .;
    . += 0x2000; /* 2 pages */

    __kips_start = .;
    .kips : {
        KEEP(*(.kips))
    }
    . = ALIGN(8);
    __kips_end = .;
}
