#include "vga_text.h"

static uint16_t* const vga_buffer = (uint16_t*)(uintptr_t)VGA_TEXT_ADDR;
static size_t vga_row = 0;
static size_t vga_col = 0;

void vga_text_clear(uint8_t color) {
    uint16_t blank = vga_entry(' ', color);
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = blank;
        }
    }
    vga_row = 0;
    vga_col = 0;
}

void vga_text_init(void) {
    vga_text_clear(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

static void vga_scroll(uint8_t color) {
    uint16_t blank = vga_entry(' ', color);
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
    vga_row = VGA_HEIGHT - 1;
}

void vga_text_putc(char c, uint8_t color) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
        if (vga_row >= VGA_HEIGHT) {
            vga_scroll(color);
        }
        return;
    }

    if (c == '\r') {
        vga_col = 0;
        return;
    }

    vga_buffer[vga_row * VGA_WIDTH + vga_col] = vga_entry(c, color);
    vga_col++;

    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
        if (vga_row >= VGA_HEIGHT) {
            vga_scroll(color);
        }
    }
}

void vga_text_puts(const char* str, uint8_t color) {
    if (!str) return;
    while (*str) {
        vga_text_putc(*str, color);
        str++;
    }
}
