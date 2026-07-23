#ifndef QOS_VGA_TEXT_H
#define QOS_VGA_TEXT_H

#include <stdint.h>
#include <stddef.h>

#define VGA_TEXT_ADDR 0xB8000
#define VGA_WIDTH     80
#define VGA_HEIGHT    25

/* Color attributes */
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_WHITE         15

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(char uc, uint8_t color) {
    return (uint16_t)uc | ((uint16_t)color << 8);
}

void vga_text_init(void);
void vga_text_clear(uint8_t color);
void vga_text_putc(char c, uint8_t color);
void vga_text_puts(const char* str, uint8_t color);

#endif /* QOS_VGA_TEXT_H */
