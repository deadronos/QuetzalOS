#include "ritual_geo.h"
#include "../drivers/vbe.h"

void draw_step_pyramid(uint32_t cx, uint32_t cy, uint32_t base_width, uint32_t steps) {
    uint32_t step_height = 14;
    uint32_t step_span = (steps + 1 > 0) ? (base_width / (steps + 1)) : 0;

    for (uint32_t i = 0; i < steps; i++) {
        uint32_t width_deduction = i * step_span;
        uint32_t w = (base_width > width_deduction) ? (base_width - width_deduction) : 0;
        uint32_t half_w = w / 2;
        uint32_t x = (cx >= half_w) ? (cx - half_w) : 0;

        uint32_t total_step_offset = (i + 1) * step_height;
        if (cy < total_step_offset) continue;
        uint32_t y = cy - total_step_offset;

        uint32_t color = (i % 2 == 0) ? COLOR_AZTEC_GOLD : COLOR_JADE;
        vbe_fill_rect(x, y, w, step_height, color);
    }

    /* Temple Sanctuary on top */
    uint32_t top_w = 32;
    uint32_t top_h = 20;
    uint32_t temple_offset = (steps * step_height) + top_h;
    if (cy >= temple_offset) {
        uint32_t top_x = (cx >= (top_w / 2)) ? (cx - (top_w / 2)) : 0;
        uint32_t top_y = cy - temple_offset;
        vbe_fill_rect(top_x, top_y, top_w, top_h, COLOR_TURQUOISE);
    }
}

void draw_feathered_serpent(uint32_t start_x, uint32_t start_y, uint32_t length, uint64_t tick) {
    for (uint32_t i = 0; i < length; i++) {
        /* Wave calculation based on tick */
        int wave = ((i + (tick / 5)) % 8 < 4) ? 4 : -4;
        uint32_t x = start_x + (i * 12);
        uint32_t y = (wave < 0 && (uint32_t)(-wave) > start_y) ? 0 : (uint32_t)((int)start_y + wave);

        uint32_t color = (i == length - 1) ? COLOR_COCHINEAL_RED : ((i % 2 == 0) ? COLOR_JADE : COLOR_AZTEC_GOLD);
        vbe_fill_rect(x, y, 10, 10, color);
    }
}

void draw_sun_disk(uint32_t cx, uint32_t cy, uint32_t radius, uint64_t tick) {
    (void)tick;
    /* Outer solar disc in Aztec Gold */
    vbe_fill_circle(cx, cy, radius, COLOR_AZTEC_GOLD);

    /* Inner obsidian solar ring */
    if (radius > 6) {
        vbe_fill_circle(cx, cy, radius - 6, COLOR_OBSIDIAN);
    }

    /* Core solar center in Cochineal Red */
    if (radius > 12) {
        vbe_fill_circle(cx, cy, radius / 2, COLOR_COCHINEAL_RED);
    }

    /* Outer golden corona ring outline */
    vbe_draw_circle(cx, cy, radius, COLOR_AZTEC_GOLD);
}
