ENTRY(_start)

BOOT_PHYS_BASE   = 0x00080000;

SECTIONS
{
    . = BOOT_PHYS_BASE;
    .text : 
    {
        __start = .;
        __text_start = .;
        KEEP(*(.text.boot))
        *(.text.*)
        . = ALIGN(4096);
        __text_end = .;
    }

    .rodata :
    {
        __rodata_start = .;
        *(.rodata .rodata.*)
        . = ALIGN(4096);
        __rodata_end = .;
    }
    . = ALIGN(4096);
    
    PROVIDE(_data = .);
    .data.kernel : 
    {
        __kernel_blob = .;
        *(.data.kernel)
    }
    .data :
    {
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(4096);
        __data_end = .;
    }

    .bss :
    {
        __bss_start = .;
        *(.bss .COMMON)
        . = ALIGN(4096);
        __bss_end = .;
    }

    __bss_size = __bss_end - __bss_start;
    . = ALIGN(4096);
    __stack_bottom = .;
    . = . + 0x1000; /* 4KB stack for core 0 */
    __stack_top = .;
    . = ALIGN(4096);
}
