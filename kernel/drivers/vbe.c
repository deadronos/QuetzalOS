#include "vbe.h"
#include "../memory_map.h"
#include "../arch/x86_64/io.h"
#include "../graphics/font8x8.h"

#include <stddef.h>  // size_t — freestanding header, always available.

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA  0x01CF
#define VBE_DISPI_INDEX_XRES   1
#define VBE_DISPI_INDEX_YRES   2
#define VBE_DISPI_INDEX_BPP    3
#define VBE_DISPI_INDEX_ENABLE 4
#define VBE_DISPI_ENABLED      0x01
#define VBE_DISPI_LFB_ENABLED  0x40

// --- PCI probing: known-good vendor IDs for QEMU/Bochs and common cards ----
#define PCI_VENDOR_QEMU_BAR    0x1234u   // QEMU/Bochs stdvga
#define PCI_VENDOR_INTEL       0x8086u
#define PCI_VENDOR_CIRRUS      0x1013u

// Initialised to defaults from memory_map.h; overridable via vbe_init().
static uint32_t* lfb = (uint32_t*)(uintptr_t)QOS_DEFAULT_LFB_PHYS;
static uint32_t* backbuffer = (uint32_t*)(uintptr_t)QOS_BACKBUFFER_BASE;
static uint32_t fb_w = QOS_DEFAULT_FB_WIDTH;
static uint32_t fb_h = QOS_DEFAULT_FB_HEIGHT;
static uint32_t fb_pitch_pixels = QOS_DEFAULT_FB_PITCH / 4u;

static uint32_t pci_read_config(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(0xCF8, address);
    return inl(0xCFC);
}

static uint32_t pci_find_vga_lfb(void) {
    for (uint8_t dev = 0; dev < 32; dev++) {
        uint32_t vendor_device = pci_read_config(0, dev, 0, 0x00);
        uint16_t vendor = vendor_device & 0xFFFF;

        if (vendor == PCI_VENDOR_QEMU_BAR ||
            vendor == PCI_VENDOR_INTEL   ||
            vendor == PCI_VENDOR_CIRRUS) {
            uint32_t bar0 = pci_read_config(0, dev, 0, 0x10);
            if (bar0 != 0 && !(bar0 & 1)) {
                return bar0 & 0xFFFFFFF0;
            }
        }
    }
    return QOS_DEFAULT_LFB_PHYS;
}

static void bochs_vbe_set_mode(uint16_t width, uint16_t height, uint16_t bpp) {
    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_ENABLE);
    outw(VBE_DISPI_IOPORT_DATA, 0); // Disable VBE

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

    /* Dynamically probe PCI bus for VGA LFB address if not provided by multiboot */
    if (lfb_addr != 0) {
        lfb = (uint32_t*)(uintptr_t)lfb_addr;
    } else {
        uint32_t probed_lfb = pci_find_vga_lfb();
        if (probed_lfb != 0) {
            lfb = (uint32_t*)(uintptr_t)probed_lfb;
        }
    }

    if (width > 0)  fb_w = width;
    if (height > 0) fb_h = height;
    if (pitch > 0)  fb_pitch_pixels = pitch / 4u;

    /* Program Bochs/QEMU VBE DISPI graphics hardware */
    bochs_vbe_set_mode((uint16_t)fb_w, (uint16_t)fb_h, 32);
}

uint32_t vbe_get_width(void)  { return fb_w; }
uint32_t vbe_get_height(void) { return fb_h; }

void vbe_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x < fb_w && y < fb_h) {
        backbuffer[y * fb_pitch_pixels + x] = color;
    }
}

void vbe_clear(uint32_t color) {
    /* Pack the 32-bit colour into a 64-bit fill word so `rep stosq` writes 8
     * bytes per iteration. For a uniform fill this is roughly 8x faster than
     * a byte-by-byte loop and avoids depending on libc memset (the kernel is
     * freestanding). The trailing-byte remainder (framebuffer bytes not a
     * multiple of 8) is handled by a short stosb for the remainder.          */
    const uint64_t fill_qword = ((uint64_t)color << 32) | (uint64_t)color;
    size_t total = (size_t)fb_h * fb_pitch_pixels * sizeof(uint32_t);
    size_t quads = total / 8u;
    size_t tail  = total % 8u;

    void* dst = backbuffer;
    __asm__ volatile (
        "cld\n\t"
        "rep stosq"
        : "+D"(dst), "+c"(quads)
        : "a"(fill_qword)
        : "memory"
    );

    if (tail >= 4u) {
        uint32_t* tail_ptr = (uint32_t*)((uint8_t*)backbuffer + quads * 8u);
        *tail_ptr = color;
    }
}

void vbe_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    /* Hoist the bounds clip out of the inner loop. */
    if (x >= fb_w || y >= fb_h) return;
    uint32_t x_end = x + w;
    uint32_t y_end = y + h;
    if (x_end > fb_w || x_end < x) x_end = fb_w;
    if (y_end > fb_h || y_end < y) y_end = fb_h;

    for (uint32_t py = y; py < y_end; py++) {
        uint32_t* row = &backbuffer[py * fb_pitch_pixels + x];
        for (uint32_t px = 0; px < (x_end - x); px++) {
            row[px] = color;
        }
    }
}

void vbe_draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color) {
    /* Bresenham line algorithm. Handles all octants; integer-only. */
    int dx = (int)(x1 > x0 ? x1 - x0 : x0 - x1);
    int dy = (int)(y1 > y0 ? y1 - y0 : y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int x = (int)x0;
    int y = (int)y0;
    for (;;) {
        /* Bounds check via draw_pixel which clips to fb_w/fb_h. */
        vbe_draw_pixel((uint32_t)x, (uint32_t)y, color);
        if (x == (int)x1 && y == (int)y1) break;
        int e2 = err << 1;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 <  dx) { err += dx; y += sy; }
    }
}

void vbe_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    uint8_t idx = (uint8_t)c;
    if (idx >= 128u) idx = '?';

    /* Skip the glyph if every row is zero (e.g. undefined control code). */
    int any_pixel = 0;
    for (int row = 0; row < 8; row++) {
        if (font8x8_basic[idx][row] != 0) { any_pixel = 1; break; }
    }
    if (!any_pixel && bg == COLOR_TRANSPARENT) return;

    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[idx][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1u << (7 - col))) {
                vbe_draw_pixel(x + (uint32_t)col, y + (uint32_t)row, fg);
            } else if (bg != COLOR_TRANSPARENT) {
                vbe_draw_pixel(x + (uint32_t)col, y + (uint32_t)row, bg);
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
    /* Copy backbuffer -> LFB using `rep movsq`. This is the canonical kernel
     * blit path: avoids the libc memcpy dependency (we're freestanding) and
     * is faster than a hand-rolled quad loop because the microcode handles
     * the streaming write-combine hints.                                     */
    size_t total = (size_t)fb_h * fb_pitch_pixels * sizeof(uint32_t);
    size_t quads = total / 8u;
    size_t tail  = total % 8u;

    void*       dst = lfb;
    const void* src = backbuffer;
    __asm__ volatile (
        "cld\n\t"
        "rep movsq"
        : "+D"(dst), "+S"(src), "+c"(quads)
        :
        : "memory"
    );

    if (tail) {
        dst = (uint8_t*)lfb + quads * 8u;
        src = (const uint8_t*)backbuffer + quads * 8u;
        __asm__ volatile (
            "rep movsb"
            : "+D"(dst), "+S"(src), "+c"(tail)
            :
            : "memory"
        );
    }
}
