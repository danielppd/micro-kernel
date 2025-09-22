# Tenta usar i386-elf-gcc se existir; senão usa gcc com -m32
CC := $(shell which i386-elf-gcc 2>/dev/null || echo gcc)
AS := nasm
LD := $(shell which i386-elf-ld 2>/dev/null || echo ld)
CFLAGS := -O2 -ffreestanding -Wall -Wextra -std=gnu11 -fno-stack-protector -fno-pic -m32
LDFLAGS := -T linker.ld -nostdlib -z max-page-size=0x1000 -m elf_i386

KOBJ := kernel/vga.o kernel/util.o kernel/idt.o kernel/isr.o kernel/irq.o kernel/keyboard.o kernel/kernel.o

all: build/kernel.bin

build:
	@mkdir -p build

build/kernel.bin: build/boot.o $(KOBJ) linker.ld | build
	$(LD) $(LDFLAGS) -o $@ build/boot.o $(KOBJ)
	@echo "[LD] $@"

build/boot.o: boot.s | build
	$(AS) -f elf32 boot.s -o $@
	@echo "[AS] $@"

kernel/%.o: kernel/%.c kernel/%.h
	$(CC) $(CFLAGS) -Ikernel -c $< -o $@
	@echo "[CC] $@"

kernel/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) -Ikernel -c $< -o $@
	@echo "[CC] $@"

iso: all
	@rm -rf iso/boot
	@mkdir -p iso/boot/grub
	@cp build/kernel.bin iso/boot/kernel.bin
	@grub-mkrescue -o build/os.iso iso 2>/dev/null || \
		grub2-mkrescue -o build/os.iso iso
	@echo "[ISO] build/os.iso"

run: all
	qemu-system-i386 -kernel build/kernel.bin

run-iso: iso
	qemu-system-i386 -cdrom build/os.iso

clean:
	rm -rf build $(KOBJ)

.PHONY: all iso run run-iso clean
