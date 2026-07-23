#include "ritual_geo.h"
#include "../drivers/vbe.h"

void draw_step_pyramid(uint32_t cx, uint32_t cy, uint32_t base_width, uint32_t steps) {
    uint32_t step_height = 14;
    for (uint32_t i = 0; i < steps; i++) {
        uint32_t w = base_width - (i * (base_width / (steps + 1)));
        uint32_t x = cx - (w / 2);
        uint32_t y = cy - (i * step_height);

        uint32_t color = (i % 2 == 0) ? COLOR_AZTEC_GOLD : COLOR_JADE;
        vbe_fill_rect(x, y - step_height, w, step_height, color);
    }

    /* Temple Sanctuary on top */
    uint32_t top_w = 32;
    uint32_t top_h = 20;
    vbe_fill_rect(cx - (top_w / 2), cy - (steps * step_height) - top_h, top_w, top_h, COLOR_TURQUOISE);
}

void draw_feathered_serpent(uint32_t start_x, uint32_t start_y, uint32_t length, uint64_t tick) {
    for (uint32_t i = 0; i < length; i++) {
        /* Wave calculation based on tick */
        int wave = ((i + (tick / 5)) % 8 < 4) ? 4 : -4;
        uint32_t x = start_x + (i * 12);
        uint32_t y = start_y + wave;

        uint32_t color = (i == length - 1) ? COLOR_COCHINEAL_RED : ((i % 2 == 0) ? COLOR_JADE : COLOR_AZTEC_GOLD);
        vbe_fill_rect(x, y, 10, 10, color);
    }
}

void draw_sun_disk(uint32_t cx, uint32_t cy, uint32_t radius, uint64_t tick) {
    (void)tick;
    /* Draw outer solar ring */
    vbe_fill_rect(cx - radius, cy - radius, radius * 2, radius * 2, COLOR_AZTEC_GOLD);
    vbe_fill_rect(cx - radius + 4, cy - radius + 4, (radius - 4) * 2, (radius - 4) * 2, COLOR_OBSIDIAN);
    vbe_fill_rect(cx - (radius / 2), cy - (radius / 2), radius, radius, COLOR_COCHINEAL_RED);
}
