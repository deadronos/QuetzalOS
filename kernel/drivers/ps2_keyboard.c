#include "ps2_keyboard.h"
#include "../arch/x86_64/io.h"

#include <stdatomic.h>

#define KBD_BUFFER_SIZE 256

static char kbd_buffer[KBD_BUFFER_SIZE];
static _Atomic uint16_t kbd_head = 0;
static _Atomic uint16_t kbd_tail = 0;

static const char scancode_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',   0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

void keyboard_init(void) {
    atomic_store_explicit(&kbd_head, 0, memory_order_relaxed);
    atomic_store_explicit(&kbd_tail, 0, memory_order_relaxed);
}

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    /* Check if key release (bit 7 set) */
    if (!(scancode & 0x80)) {
        char ch = scancode_ascii[scancode];
        if (ch) {
            uint16_t head = atomic_load_explicit(&kbd_head, memory_order_relaxed);
            uint16_t tail = atomic_load_explicit(&kbd_tail, memory_order_relaxed);
            uint16_t next = (head + 1) % KBD_BUFFER_SIZE;
            if (next != tail) {
                kbd_buffer[head] = ch;
                atomic_store_explicit(&kbd_head, next, memory_order_release);
            }
        }
    }

    /* Send EOI to Master PIC */
    outb(0x20, 0x20);
}

int keyboard_has_char(void) {
    uint16_t head = atomic_load_explicit(&kbd_head, memory_order_relaxed);
    uint16_t tail = atomic_load_explicit(&kbd_tail, memory_order_relaxed);
    return head != tail;
}

char keyboard_getchar(void) {
    uint16_t head = atomic_load_explicit(&kbd_head, memory_order_acquire);
    uint16_t tail = atomic_load_explicit(&kbd_tail, memory_order_relaxed);
    if (head == tail) return 0;
    char ch = kbd_buffer[tail];
    uint16_t next = (tail + 1) % KBD_BUFFER_SIZE;
    atomic_store_explicit(&kbd_tail, next, memory_order_release);
    return ch;
}
