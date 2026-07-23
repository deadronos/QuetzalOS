#include "ps2_mouse.h"
#include "vbe.h"
#include "../arch/x86_64/io.h"

#include <stdatomic.h>

static _Atomic uint32_t mouse_x = 400;
static _Atomic uint32_t mouse_y = 300;
static _Atomic uint8_t mouse_buttons = 0;

static uint8_t mouse_cycle = 0;
static uint8_t mouse_bytes[3];

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return;
        }
    } else {
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return;
        }
    }
}

static void mouse_write(uint8_t write_byte) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, write_byte);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_init(void) {
    uint32_t sw = vbe_get_width();
    uint32_t sh = vbe_get_height();
    atomic_store(&mouse_x, sw / 2);
    atomic_store(&mouse_y, sh / 2);
    atomic_store(&mouse_buttons, 0);

    mouse_cycle = 0;

    /* Enable Auxiliary Mouse Device */
    mouse_wait(1);
    outb(0x64, 0xA8);

    /* Enable IRQ12 Mouse Interrupts in 8042 Controller Command Byte */
    mouse_wait(1);
    outb(0x64, 0x20);
    uint8_t status = (mouse_read() | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    /* Reset mouse to default settings & enable streaming data reporting */
    mouse_write(0xF6);
    mouse_read(); // ACK (0xFA)

    mouse_write(0xF4);
    mouse_read(); // ACK (0xFA)
}

void mouse_handler(void) {
    uint8_t data = inb(0x60);

    switch (mouse_cycle) {
        case 0:
            if (data & 0x08) { // Bit 3 of packet byte 0 is always 1 in PS/2 mouse packets
                mouse_bytes[0] = data;
                mouse_cycle = 1;
            }
            break;
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle = 2;
            break;
        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 0;

            uint8_t flags = mouse_bytes[0];
            int rel_x = (int8_t)mouse_bytes[1];
            int rel_y = (int8_t)mouse_bytes[2];

            /* Invert Y movement because PS/2 mouse reports positive UP */
            int new_x = (int)atomic_load(&mouse_x) + rel_x;
            int new_y = (int)atomic_load(&mouse_y) - rel_y;

            uint32_t sw = vbe_get_width();
            uint32_t sh = vbe_get_height();

            if (new_x < 0) new_x = 0;
            if ((uint32_t)new_x >= sw) new_x = (sw > 0) ? (sw - 1) : 0;
            if (new_y < 0) new_y = 0;
            if ((uint32_t)new_y >= sh) new_y = (sh > 0) ? (sh - 1) : 0;

            atomic_store(&mouse_x, (uint32_t)new_x);
            atomic_store(&mouse_y, (uint32_t)new_y);
            atomic_store(&mouse_buttons, flags & 0x07);
            break;
        default:
            mouse_cycle = 0;
            break;
    }

    /* Send EOI to both Slave PIC (0xA0) and Master PIC (0x20) */
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void mouse_get_position(uint32_t* out_x, uint32_t* out_y) {
    if (out_x) *out_x = atomic_load(&mouse_x);
    if (out_y) *out_y = atomic_load(&mouse_y);
}

uint8_t mouse_get_buttons(void) {
    return atomic_load(&mouse_buttons);
}

/* Custom Feathered Cursor Bitmap (10 x 14) */
static const uint16_t cursor_sprite[14] = {
    0b1000000000000000,
    0b1100000000000000,
    0b1110000000000000,
    0b1111000000000000,
    0b1111100000000000,
    0b1111110000000000,
    0b1111111000000000,
    0b1111111100000000,
    0b1111111110000000,
    0b1111110000000000,
    0b1101111000000000,
    0b1000111000000000,
    0b0000011100000000,
    0b0000001100000000,
};

void mouse_draw_cursor(void) {
    uint32_t cx = atomic_load(&mouse_x);
    uint32_t cy = atomic_load(&mouse_y);
    uint8_t btns = atomic_load(&mouse_buttons);

    uint32_t fg_color = (btns & 1) ? COLOR_COCHINEAL_RED : COLOR_JADE;
    uint32_t border_color = COLOR_AZTEC_GOLD;

    for (int row = 0; row < 14; row++) {
        uint16_t row_bits = cursor_sprite[row];
        for (int col = 0; col < 10; col++) {
            if (row_bits & (1u << (15 - col))) {
                uint32_t px = cx + (uint32_t)col;
                uint32_t py = cy + (uint32_t)row;
                uint32_t color = (col == 0 || row == 0 || col == row) ? border_color : fg_color;
                vbe_draw_pixel(px, py, color);
            }
        }
    }
}
