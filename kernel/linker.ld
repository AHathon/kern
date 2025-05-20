ENTRY(__start_kernel)

BOOT_PHYS_BASE = 0x00080000;
KERNEL_BASE    = 0xFFFFFF8000000000;
PREKERNEL_SIZE = 0x1000;

SECTIONS
{
    . = BOOT_PHYS_BASE + PREKERNEL_SIZE;
    .text : 
    {
        __start_kernel = .;
        __text_start = .;
        KEEP(*(.text.kernel))
        *(.text.kernel)
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
    __page_table = .;
    . += 0x2000; /* 2 pages */

    .kips : {
        __kips_start = .;
        KEEP(*(.kips))
        . = ALIGN(8);
        __kips_end = .;
    }
}
