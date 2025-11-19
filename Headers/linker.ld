ENTRY(_start)

SECTIONS {
    . = 0x00010000;      /* adresa la care l-a încărcat bootloader-ul */

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }
}
