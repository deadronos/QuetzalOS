#include "kernel.h"
#include "memory_map.h"
#include "arch/x86_64/idt.h"
#include "drivers/vbe.h"
#include "drivers/pit.h"
#include "drivers/ps2_keyboard.h"
#include "graphics/ritual_geo.h"
#include "serpentc/serpentc.h"

static char num_to_char(uint32_t val) {
    if (val < 10) return '0' + val;
    return '0' + (val % 10);
}

void kernel_main(multiboot_info_t* mb_info) {
    uint64_t fb_addr = QOS_DEFAULT_LFB_PHYS;
    uint32_t width   = QOS_DEFAULT_FB_WIDTH;
    uint32_t height  = QOS_DEFAULT_FB_HEIGHT;
    uint32_t pitch   = QOS_DEFAULT_FB_PITCH;

    if (mb_info && (mb_info->flags & (1u << 12)) && mb_info->framebuffer_addr) {
        fb_addr = mb_info->framebuffer_addr;
        width   = mb_info->framebuffer_width;
        height  = mb_info->framebuffer_height;
        pitch   = mb_info->framebuffer_pitch;
    }

    /* Subsystem Initializations */
    vbe_init(fb_addr, width, height, pitch, 32);
    idt_init();
    pit_init(100); // 100 Hz timer
    keyboard_init();
    serpentc_init();

    /* Enable Interrupts */
    __asm__ volatile ("sti");

    char key_log[64] = "KEYS: ";
    int key_pos = 6;

    while (1) {
        uint64_t ticks = pit_get_ticks();
        tonalpohualli_time_t tonal = pit_get_tonal_time();

        /* Clear Backbuffer with Obsidian background */
        vbe_clear(COLOR_OBSIDIAN);

        /* Render Banner Header */
        vbe_fill_rect(0, 0, width, 40, COLOR_JADE);
        vbe_draw_string(16, 12, "QUETZAL-OS v0.1 - PRAISE THE FEATHERED SERPENT [RING 0 / SAS 64-BIT]", COLOR_FEATHER_WHITE, COLOR_JADE);

        /* Render Ritual Geometry Elements */
        draw_step_pyramid(width / 2, (height / 2) + 60, 240, 6);
        draw_sun_disk(width / 2, 120, 36, ticks);
        draw_feathered_serpent(60, 440, 16, ticks);

        /* Render Sacred Tonalpohualli Calendar Widget */
        vbe_fill_rect(16, 60, 340, 90, COLOR_AZTEC_GOLD);
        vbe_fill_rect(18, 62, 336, 86, COLOR_OBSIDIAN);
        vbe_draw_string(28, 70, "=== TONALPOHUALLI SACRED TIME ===", COLOR_AZTEC_GOLD, COLOR_OBSIDIAN);

        vbe_draw_string(28, 92, "TRECENA NUMBER:", COLOR_TURQUOISE, COLOR_OBSIDIAN);
        char t_str[4] = { num_to_char(tonal.trecena_num / 10), num_to_char(tonal.trecena_num % 10), '\0', '\0' };
        vbe_draw_string(160, 92, t_str, COLOR_FEATHER_WHITE, COLOR_OBSIDIAN);

        vbe_draw_string(28, 112, "DAY SIGN:", COLOR_TURQUOISE, COLOR_OBSIDIAN);
        vbe_draw_string(104, 112, tonal.sign_name, COLOR_JADE, COLOR_OBSIDIAN);

        /* Render a diagonal ritual line (calls the new vbe_draw_line that
         * serpentc_eval advertised but never implemented).                   */
        vbe_draw_line(0, 480, 800, 580, COLOR_TURQUOISE);

        /* Handle Input */
        while (keyboard_has_char()) {
            char ch = keyboard_getchar();
            if (ch && key_pos < 60) {
                key_log[key_pos++] = ch;
                key_log[key_pos] = '\0';
            }
        }
        vbe_draw_string(16, 520, key_log, COLOR_AZTEC_GOLD, COLOR_OBSIDIAN);

        /* Evaluate SerpentC Script sample (still a stub; see docs/IMPROVEMENTS.md) */
        serpentc_eval("feather_draw_line(0,0,800,600);");

        /* Swap double buffer to physical framebuffer */
        vbe_swap_buffers();

        __asm__ volatile ("hlt");
    }
}
