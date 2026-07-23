#ifndef QUETZAL_VBE_H
#define QUETZAL_VBE_H

#include <stdint.h>

#define COLOR_OBSIDIAN       0x000B0B0E
#define COLOR_JADE           0x0000A86B
#define COLOR_AZTEC_GOLD     0x00C59B27
#define COLOR_TURQUOISE      0x0040E0D0
#define COLOR_COCHINEAL_RED  0x009E0027
#define COLOR_FEATHER_WHITE  0x00F0F8FF

void vbe_init(uint64_t lfb_addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp);
void vbe_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void vbe_clear(uint32_t color);
void vbe_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void vbe_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void vbe_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t fg, uint32_t bg);
void vbe_swap_buffers(void);

uint32_t vbe_get_width(void);
uint32_t vbe_get_height(void);

#endif /* QUETZAL_VBE_H */
