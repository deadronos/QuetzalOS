#ifndef QUETZAL_IDT_H
#define QUETZAL_IDT_H

#include <stdint.h>

typedef struct {
    uint16_t offset_low;    // Offset bits 0..15
    uint16_t selector;      // Target code segment selector
    uint8_t  ist;           // Interrupt Stack Table offset (bits 0..2)
    uint8_t  type_attributes; // Type and attributes (Present, DPL, Gate Type)
    uint16_t offset_mid;    // Offset bits 16..31
    uint32_t offset_high;   // Offset bits 32..63
    uint32_t zero;          // Reserved
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_ptr_t;

void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);

#endif /* QUETZAL_IDT_H */
