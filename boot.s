; boot.s — cabeçalho multiboot + pequeno stub que chama kernel_main()
; Montagem: nasm -f elf32 boot.s -o build/boot.o


BITS 32


SECTION .multiboot
align 4
MB_MAGIC equ 0x1BADB002
MB_FLAGS equ 0x00000003 ; align + memory info
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)


dd MB_MAGIC
dd MB_FLAGS
dd MB_CHECKSUM


SECTION .text
extern kernel_main


global start
start:
; Desabilita interrupções no stub inicial
cli
; Pilha simples
mov esp, stack_top
call kernel_main
.hang:
hlt
jmp .hang


SECTION .bss
align 16
stack_bottom:
resb 4096
stack_top: