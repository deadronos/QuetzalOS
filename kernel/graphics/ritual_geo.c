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
    /* Draw outer solar ring with origin underflow checks */
    uint32_t min_x = (cx >= radius) ? (cx - radius) : 0;
    uint32_t min_y = (cy >= radius) ? (cy - radius) : 0;
    vbe_fill_rect(min_x, min_y, radius * 2, radius * 2, COLOR_AZTEC_GOLD);

    if (radius > 4) {
        uint32_t inner_rad = radius - 4;
        uint32_t inner_x = (cx >= inner_rad) ? (cx - inner_rad) : 0;
        uint32_t inner_y = (cy >= inner_rad) ? (cy - inner_rad) : 0;
        vbe_fill_rect(inner_x, inner_y, inner_rad * 2, inner_rad * 2, COLOR_OBSIDIAN);
    }

    uint32_t half_rad = radius / 2;
    uint32_t center_x = (cx >= half_rad) ? (cx - half_rad) : 0;
    uint32_t center_y = (cy >= half_rad) ? (cy - half_rad) : 0;
    vbe_fill_rect(center_x, center_y, radius, radius, COLOR_COCHINEAL_RED);
}
