OUTPUT_FORMAT("elf64-x86-64")
ENTRY(efi_main)

SECTIONS
{
    . = 0;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss (NOLOAD) : {
        *(COMMON)
        *(.bss*)
    }
}
