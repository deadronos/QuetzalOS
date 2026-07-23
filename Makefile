# OS Detection
ifeq ($(OS),Windows_NT)
    HOST_OS := Windows
    RM := del /F /Q 2>nul || true
    ifneq ($(shell where rm 2>nul),)
        RM := rm -f
    endif
else
    HOST_OS := $(shell uname -s)
    RM := rm -f
    export PATH := /opt/homebrew/bin:/usr/local/bin:$(PATH)
endif

# Toolchain definitions
CROSS_GCC ?= x86_64-elf-gcc
CROSS_LD  ?= x86_64-elf-ld
OBJCOPY   ?= x86_64-elf-objcopy
NASM      ?= nasm

# Check toolchain availability
ifeq ($(shell $(CROSS_GCC) --version 2>/dev/null),)
    CC = clang -target x86_64-unknown-elf
    LD = ld
    OBJCOPY = objcopy
else
    CC = $(CROSS_GCC)
    LD = $(CROSS_LD)
endif

AS = $(NASM)

CFLAGS  = -m64 -ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -nostdlib -fno-builtin -Wall -Wextra -O2
ASFLAGS = -f elf64

OBJS = boot/boot.o \
       kernel/arch/x86_64/idt.o \
       kernel/arch/x86_64/isr.o \
       kernel/arch/x86_64/serial.o \
       kernel/drivers/vga_text.o \
       kernel/drivers/vbe.o \
       kernel/drivers/pit.o \
       kernel/drivers/ps2_keyboard.o \
       kernel/graphics/font8x8.o \
       kernel/graphics/ritual_geo.o \
       kernel/graphics/console.o \
       kernel/graphics/widget.o \
       kernel/serpentc/lexer.o \
       kernel/serpentc/builtins.o \
       kernel/serpentc/serpentc.o \
       kernel/mm/phys.o \
       kernel/kernel.o

all: quetzal.bin

boot/boot.o: boot/boot.asm
	$(AS) $(ASFLAGS) boot/boot.asm -o boot/boot.o

kernel/arch/x86_64/isr.o: kernel/arch/x86_64/isr.asm
	$(AS) $(ASFLAGS) kernel/arch/x86_64/isr.asm -o kernel/arch/x86_64/isr.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

quetzal.elf: $(OBJS) linker.ld
	$(LD) -n -T linker.ld -o quetzal.elf $(OBJS)

quetzal.bin: quetzal.elf
	$(OBJCOPY) -O elf32-i386 quetzal.elf quetzal.bin

clean:
	$(RM) boot/*.o kernel/*.o kernel/*/*.o kernel/*/*/*.o quetzal.elf quetzal.bin

.PHONY: all clean
