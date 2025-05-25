ENTRY(__start_kernel)

BOOT_PHYS_BASE = 0x00080000;
KERNEL_BASE    = 0xFFFFFF8000000000;
PREKERNEL_SIZE = 0x2000;

SECTIONS
{
    . = KERNEL_BASE + BOOT_PHYS_BASE + PREKERNEL_SIZE;
    .text : ALIGN(4K)
    {
        __start_kernel = .;
        __text_start = .;
        KEEP(*(.text.kernel))
        *(.text .text.*)
        __text_end = .;
    }

    .rodata : ALIGN(4K)
    {
        __rodata_start = .;
        *(.rodata .rodata.*)
        __rodata_end = .;
    }
    . = ALIGN(4096);
    
    PROVIDE(_data = .);
    .data : ALIGN(4K)
    {
        __data_start = .;
        *(.data .data.*)
        __data_end = .;
    }

    .bss : ALIGN(4K)
    {
        __bss_start = .;
        *(.bss .COMMON)
        __bss_end = .;
    }
    __bss_size = __bss_end - __bss_start;
    . = ALIGN(4096);
    __stack_bottom = .;
    . = . + 0x1000; /* 4KB stack for core 0 */
    __stack_top = .;
    . = ALIGN(4096);
    __page_table = .;
    . += 0x2000; /* 2 pages */
    
    .kips : ALIGN(4K)
    {
        __kips_start = .;
        KEEP(*(.kips))
        __kips_end = .;
    }
}
