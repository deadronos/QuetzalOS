# Explicitly include Homebrew paths on macOS ARM64 / x86_64
export PATH := /opt/homebrew/bin:/usr/local/bin:$(PATH)

# Detect toolchain binaries
CROSS_GCC := $(shell which x86_64-elf-gcc 2>/dev/null)
CROSS_LD  := $(shell which x86_64-elf-ld 2>/dev/null)
OBJCOPY   := $(shell which x86_64-elf-objcopy 2>/dev/null)
NASM      := $(shell which nasm 2>/dev/null)

ifneq ($(CROSS_GCC),)
    CC = $(CROSS_GCC)
    LD = $(CROSS_LD)
else
    CC = clang -target x86_64-unknown-elf
    LD = ld
    OBJCOPY = objcopy
endif

AS = $(NASM)

CFLAGS  = -m64 -ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -nostdlib -fno-builtin -Wall -Wextra -O2
ASFLAGS = -f elf64

OBJS = boot/boot.o \
       kernel/arch/x86_64/idt.o \
       kernel/arch/x86_64/isr.o \
       kernel/drivers/vbe.o \
       kernel/drivers/pit.o \
       kernel/drivers/ps2_keyboard.o \
       kernel/graphics/font8x8.o \
       kernel/graphics/ritual_geo.o \
       kernel/serpentc/serpentc.o \
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
	rm -f boot/*.o kernel/*.o kernel/*/*.o kernel/*/*/*.o quetzal.elf quetzal.bin

.PHONY: all clean
