#include "ps2_keyboard.h"
#include "../arch/x86_64/io.h"

#define KBD_BUFFER_SIZE 256

static char kbd_buffer[KBD_BUFFER_SIZE];
static uint16_t kbd_head = 0;
static uint16_t kbd_tail = 0;

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
    kbd_head = 0;
    kbd_tail = 0;
}

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    /* Check if key release (bit 7 set) */
    if (!(scancode & 0x80)) {
        char ch = scancode_ascii[scancode];
        if (ch) {
            uint16_t next = (kbd_head + 1) % KBD_BUFFER_SIZE;
            if (next != kbd_tail) {
                kbd_buffer[kbd_head] = ch;
                kbd_head = next;
            }
        }
    }

    /* Send EOI to Master PIC */
    outb(0x20, 0x20);
}

int keyboard_has_char(void) {
    return kbd_head != kbd_tail;
}

char keyboard_getchar(void) {
    if (kbd_head == kbd_tail) return 0;
    char ch = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return ch;
}
