# linker.ld


ENTRY(start)
SECTIONS
{
. = 1M; /* endereço de carga padrão do GRUB */


.text : {
*(.multiboot)
*(.text*)
}


.rodata : { *(.rodata*) }
.data : { *(.data*) }
.bss : { *(.bss*) *(COMMON) }
}