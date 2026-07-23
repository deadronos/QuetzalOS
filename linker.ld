ENTRY(start)
OUTPUT_FORMAT("elf64-x86-64")

SECTIONS
{
    /* Load Kernel at 1MB physical RAM */
    . = 0x00100000;

    .text ALIGN(4K) :
    {
        *(.multiboot)
        *(.text*)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata*)
    }

    .data ALIGN(4K) :
    {
        *(.data*)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss*)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
    }
}
