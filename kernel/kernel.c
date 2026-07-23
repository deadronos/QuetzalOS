#include "kernel.h"
#include "memory_map.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/serial.h"
#include "drivers/vga_text.h"
#include "drivers/vbe.h"
#include "drivers/pit.h"
#include "drivers/ps2_keyboard.h"
#include "drivers/ps2_mouse.h"
#include "graphics/ritual_geo.h"
#include "graphics/console.h"
#include "graphics/widget.h"
#include "serpentc/serpentc.h"
#include "mm/phys.h"

static char num_to_char(uint32_t val) {
    if (val < 10) return '0' + val;
    return '0' + (val % 10);
}

static void draw_banner_widget(widget_t* w, uint64_t tick) {
    (void)tick;
    vbe_draw_string(w->x + 16, w->y + 12, w->title, COLOR_FEATHER_WHITE, COLOR_JADE);
}

static void draw_tonalpohualli_widget(widget_t* w, uint64_t tick) {
    (void)tick;
    tonalpohualli_time_t tonal = pit_get_tonal_time();
    vbe_draw_string(w->x + 12, w->y + 10, "=== TONALPOHUALLI SACRED TIME ===", COLOR_AZTEC_GOLD, COLOR_OBSIDIAN);
    vbe_draw_string(w->x + 12, w->y + 32, "TRECENA NUMBER:", COLOR_TURQUOISE, COLOR_OBSIDIAN);
    char t_str[4] = { num_to_char(tonal.trecena_num / 10), num_to_char(tonal.trecena_num % 10), '\0', '\0' };
    vbe_draw_string(w->x + 144, w->y + 32, t_str, COLOR_FEATHER_WHITE, COLOR_OBSIDIAN);
    vbe_draw_string(w->x + 12, w->y + 52, "DAY SIGN:", COLOR_TURQUOISE, COLOR_OBSIDIAN);
    vbe_draw_string(w->x + 88, w->y + 52, tonal.sign_name, COLOR_JADE, COLOR_OBSIDIAN);
}

static void draw_ritual_geo_widget(widget_t* w, uint64_t tick) {
    uint32_t cx = w->x + (w->w / 2);
    draw_step_pyramid(cx, w->y + 190, 240, 6);
    draw_sun_disk(cx, w->y + 20, 36, tick);
    draw_feathered_serpent(w->x + 40, w->y + 230, 16, tick);
}

void kernel_main(multiboot_info_t* mb_info) {
    /* Initialize Serial COM1 Logger & VGA Text Diagnostic Fallback */
    serial_init();
    vga_text_init();
    kprintf("[KERNEL] QuetzalOS v0.1 Long Mode Booting...\n");

    /* Initialize Physical Memory Frame Allocator */
    phys_init(mb_info);

    uint64_t fb_addr = 0;
    uint32_t width   = QOS_DEFAULT_FB_WIDTH;
    uint32_t height  = QOS_DEFAULT_FB_HEIGHT;
    uint32_t pitch   = QOS_DEFAULT_FB_PITCH;

    /* Validate Multiboot framebuffer: must be flag-present, non-zero addr, Direct RGB (type 1), 32 bpp */
    if (mb_info && (mb_info->flags & (1u << 12)) && mb_info->framebuffer_addr &&
        mb_info->framebuffer_type == 1 && mb_info->framebuffer_bpp == 32) {
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
    mouse_init();
    serpentc_init();
    console_init();
    widget_system_init();

    /* Register UI Widgets */
    widget_t* w_banner = widget_create(0, 0, width, 40, "QUETZAL-OS v0.1 - PRAISE THE FEATHERED SERPENT [RING 0 / SAS 64-BIT]", draw_banner_widget);
    widget_set_style(w_banner, WIDGET_STYLE_CONTAINER, COLOR_JADE, COLOR_JADE, COLOR_JADE, COLOR_FEATHER_WHITE);

    widget_t* w_cal = widget_create(16, 60, 340, 90, "", draw_tonalpohualli_widget);
    widget_set_style(w_cal, WIDGET_STYLE_BORDERED, COLOR_OBSIDIAN, COLOR_AZTEC_GOLD, COLOR_OBSIDIAN, COLOR_AZTEC_GOLD);

    widget_t* w_geo = widget_create(16, 100, width - 32, 280, "", draw_ritual_geo_widget);
    widget_set_style(w_geo, WIDGET_STYLE_CONTAINER, COLOR_TRANSPARENT, COLOR_TRANSPARENT, COLOR_TRANSPARENT, COLOR_TRANSPARENT);

    kprintf("[KERNEL] All Subsystems Online. Framebuffer %dx%d. Enabling IRQs...\n", width, height);

    /* Enable Interrupts */
    __asm__ volatile ("sti");

    /* Initial SerpentC Script Setup */
    serpentc_eval("feather_draw_line(0, 480, 800, 580, COLOR_TURQUOISE); feather_rect(700, 50, 80, 60, COLOR_AZTEC_GOLD);");

    while (1) {
        uint64_t ticks = pit_get_ticks();

        /* Clear Backbuffer with Obsidian background */
        vbe_clear(COLOR_OBSIDIAN);

        /* Render Registered UI Widgets */
        widget_draw_all(ticks);

        /* Render static ritual baseline */
        vbe_draw_line(0, 440, 800, 455, COLOR_TURQUOISE);

        /* Handle Keyboard Input for Interactive SerpentC Console */
        while (keyboard_has_char()) {
            char ch = keyboard_getchar();
            if (ch == '`' || ch == '~') {
                console_toggle();
            } else if (ch) {
                console_handle_char(ch);
            }
        }

        /* Render Console Overlay */
        console_draw();

        /* Render Mouse Cursor Overlay */
        mouse_draw_cursor();

        /* Swap double buffer to physical framebuffer */
        vbe_swap_buffers();

        __asm__ volatile ("hlt");
    }
}
