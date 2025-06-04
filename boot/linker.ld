ENTRY(_start)

BOOT_PHYS_BASE = 0x00080000;
PREKERNEL_SIZE = 0x2000;

SECTIONS
{
    . = BOOT_PHYS_BASE;
    .text : ALIGN(4K)
    {
        __start = .;
        __text_start = .;
        KEEP(*(.text.boot))
        *(.text .text.boot)
        __text_end = .;
    }

    . = BOOT_PHYS_BASE + PREKERNEL_SIZE;
    .kernel : ALIGN(4K) {
        __kernel_blob = .;
        KEEP(*(.kernel))
        __kernel_blob_end = .;
    }

    .rodata : ALIGN(4K)
    {
        __rodata_start = .;
        *(.rodata .rodata.*)
        __rodata_end = .;
    }

    .data : ALIGN(4K)
    {
        __data_start = .;
        *(.data)
        __data_end = .;
    }

    .got : ALIGN(4K)
    {
        __got_start = .;
        *(.got .got.*)
        __got_end = .;
    }

    .bss : ALIGN(4K)
    {
        __bss_start = .;
        *(.bss .COMMON)
        __bss_end = .;
    }

    __bss_size = __bss_end - __bss_start;
    __stack_bottom = .;
    . = . + 0x1000; /* 4KB stack for core 0 */
    __stack_top = .;
    . = ALIGN(4096);
    __page_table = .;
    . += 0x2000; /* 2 pages */
    __page_table_end = .;
}
