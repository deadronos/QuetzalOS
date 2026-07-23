#include "vbe.h"
#include "../arch/x86_64/io.h"
#include "../graphics/font8x8.h"

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA  0x01CF
#define VBE_DISPI_INDEX_XRES   1
#define VBE_DISPI_INDEX_YRES   2
#define VBE_DISPI_INDEX_BPP    3
#define VBE_DISPI_INDEX_ENABLE 4
#define VBE_DISPI_ENABLED      0x01
#define VBE_DISPI_LFB_ENABLED  0x40

static uint32_t* lfb = (uint32_t*)0xFD000000;
static uint32_t* backbuffer = (uint32_t*)0x04000000;
static uint32_t fb_w = 800;
static uint32_t fb_h = 600;
static uint32_t fb_pitch_pixels = 800;

static void bochs_vbe_set_mode(uint16_t width, uint16_t height, uint16_t bpp) {
    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_ENABLE);
    outw(VBE_DISPI_IOPORT_DATA, 0); // Disable VBE first

    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_XRES);
    outw(VBE_DISPI_IOPORT_DATA, width);

    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_YRES);
    outw(VBE_DISPI_IOPORT_DATA, height);

    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_BPP);
    outw(VBE_DISPI_IOPORT_DATA, bpp);

    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_ENABLE);
    outw(VBE_DISPI_IOPORT_DATA, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
}

void vbe_init(uint64_t lfb_addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp) {
    (void)bpp;
    if (lfb_addr != 0) {
        lfb = (uint32_t*)lfb_addr;
    }
    if (width > 0) fb_w = width;
    if (height > 0) fb_h = height;
    if (pitch > 0) fb_pitch_pixels = pitch / 4;

    /* Program Bochs/QEMU VBE DISPI graphics hardware directly */
    bochs_vbe_set_mode(fb_w, fb_h, 32);
}

uint32_t vbe_get_width(void) { return fb_w; }
uint32_t vbe_get_height(void) { return fb_h; }

void vbe_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x < fb_w && y < fb_h) {
        backbuffer[y * fb_pitch_pixels + x] = color;
    }
}

void vbe_clear(uint32_t color) {
    uint32_t total = fb_h * fb_pitch_pixels;
    for (uint32_t i = 0; i < total; i++) {
        backbuffer[i] = color;
    }
}

void vbe_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t py = y; py < y + h && py < fb_h; py++) {
        for (uint32_t px = x; px < x + w && px < fb_w; px++) {
            backbuffer[py * fb_pitch_pixels + px] = color;
        }
    }
}

void vbe_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    uint8_t idx = (uint8_t)c;
    if (idx > 127) idx = '?';

    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[idx][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                vbe_draw_pixel(x + col, y + row, fg);
            } else if (bg != 0xFFFFFFFF) {
                vbe_draw_pixel(x + col, y + row, bg);
            }
        }
    }
}

void vbe_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t fg, uint32_t bg) {
    uint32_t cx = x;
    uint32_t cy = y;
    while (*str) {
        if (*str == '\n') {
            cx = x;
            cy += 10;
        } else {
            vbe_draw_char(cx, cy, *str, fg, bg);
            cx += 8;
        }
        str++;
    }
}

void vbe_swap_buffers(void) {
    uint64_t total_quads = (fb_h * fb_pitch_pixels) / 2;
    uint64_t* src = (uint64_t*)backbuffer;
    uint64_t* dst = (uint64_t*)lfb;

    for (uint64_t i = 0; i < total_quads; i++) {
        dst[i] = src[i];
    }
}
