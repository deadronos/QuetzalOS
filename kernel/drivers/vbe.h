#ifndef QUETZAL_VBE_H
#define QUETZAL_VBE_H

#include <stdint.h>

// =============================================================================
// QuetzalOS VBE / Linear Framebuffer Driver
//
// Backed by a 32-bit packed pixel backbuffer at QOS_BACKBUFFER_BASE which is
// blitted to the LFB on every vbe_swap_buffers(). See kernel/memory_map.h.
// =============================================================================

// --- Thematic palette (0x00RRGGBB) -----------------------------------------
#define COLOR_OBSIDIAN       0x000B0B0E
#define COLOR_JADE           0x0000A86B
#define COLOR_AZTEC_GOLD     0x00C59B27
#define COLOR_TURQUOISE      0x0040E0D0
#define COLOR_COCHINEAL_RED  0x009E0027
#define COLOR_FEATHER_WHITE  0x00F0F8FF

// --- Sentinel: "no background fill" ----------------------------------------
// Use this for vbe_draw_char / vbe_draw_string bg to leave background pixels
// untouched (used by the font engine to avoid overwriting pixels behind text).
#define COLOR_TRANSPARENT    0xFFFFFFFFu

void vbe_init(uint64_t lfb_addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp);
void vbe_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void vbe_clear(uint32_t color);
void vbe_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void vbe_draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color);
void vbe_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void vbe_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t fg, uint32_t bg);
void vbe_swap_buffers(void);

uint32_t vbe_get_width(void);
uint32_t vbe_get_height(void);

#endif /* QUETZAL_VBE_H */
