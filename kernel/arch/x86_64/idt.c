#include "idt.h"
#include "io.h"

static idt_entry_t idt[256];
static idt_ptr_t   idtp;

extern void irq0_stub(void);
extern void irq1_stub(void);

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = (base & 0xFFFF);
    idt[num].selector = sel;
    idt[num].ist = 0;
    idt[num].type_attributes = flags;
    idt[num].offset_mid = ((base >> 16) & 0xFFFF);
    idt[num].offset_high = ((base >> 32) & 0xFFFFFFFF);
    idt[num].zero = 0;
}

static void pic_remap(void) {
    /* Remap Master and Slave PIC vector offsets from 0x00 to 0x20 (32) and 0x28 (40) */
    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();

    outb(0x21, 0x20); // Master PIC vector offset 32
    io_wait();
    outb(0xA1, 0x28); // Slave PIC vector offset 40
    io_wait();

    outb(0x21, 0x04); // Master PIC: slave at IRQ2
    io_wait();
    outb(0xA1, 0x02); // Slave PIC: cascade identity
    io_wait();

    outb(0x21, 0x01); // 8086 mode
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    /* Mask all interrupts except IRQ0 (Timer) and IRQ1 (Keyboard) */
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
}

void idt_init(void) {
    idtp.limit = (sizeof(idt_entry_t) * 256) - 1;
    idtp.base  = (uint64_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    pic_remap();

    // IRQ0 -> Vector 32 (Timer)
    idt_set_gate(32, (uint64_t)irq0_stub, 0x08, 0x8E);
    // IRQ1 -> Vector 33 (Keyboard)
    idt_set_gate(33, (uint64_t)irq1_stub, 0x08, 0x8E);

    __asm__ volatile ("lidt %0" : : "m"(idtp));
}
