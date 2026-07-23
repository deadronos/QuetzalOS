#include "console.h"
#include "../drivers/vbe.h"
#include "../drivers/pit.h"
#include "../serpentc/serpentc.h"

static int visible = 1; // Default visible on screen
static char history[CONSOLE_MAX_HISTORY][CONSOLE_LINE_LEN];
static size_t history_count = 0;

static char prompt_buf[CONSOLE_PROMPT_MAX];
static size_t prompt_len = 0;

void console_init(void) {
    visible = 1;
    history_count = 0;
    prompt_len = 0;
    prompt_buf[0] = '\0';
    for (size_t i = 0; i < CONSOLE_MAX_HISTORY; i++) {
        history[i][0] = '\0';
    }
    console_print("QuetzalOS SerpentC REPL Console Online.");
    console_print("Type commands (e.g. feather_rect) and press Enter.");
}

void console_toggle(void) {
    visible = !visible;
}

int console_is_visible(void) {
    return visible;
}

void console_print(const char* msg) {
    if (!msg) return;

    if (history_count < CONSOLE_MAX_HISTORY) {
        size_t i = 0;
        for (; i < CONSOLE_LINE_LEN - 1 && msg[i] != '\0'; i++) {
            history[history_count][i] = msg[i];
        }
        history[history_count][i] = '\0';
        history_count++;
    } else {
        /* Shift lines up */
        for (size_t i = 0; i < CONSOLE_MAX_HISTORY - 1; i++) {
            size_t j = 0;
            for (; j < CONSOLE_LINE_LEN - 1 && history[i + 1][j] != '\0'; j++) {
                history[i][j] = history[i + 1][j];
            }
            history[i][j] = '\0';
        }
        /* Append new line to bottom slot */
        size_t i = 0;
        for (; i < CONSOLE_LINE_LEN - 1 && msg[i] != '\0'; i++) {
            history[CONSOLE_MAX_HISTORY - 1][i] = msg[i];
        }
        history[CONSOLE_MAX_HISTORY - 1][i] = '\0';
    }
}

void console_handle_char(char c) {
    if (c == '\b') {
        if (prompt_len > 0) {
            prompt_len--;
            prompt_buf[prompt_len] = '\0';
        }
    } else if (c == '\n') {
        if (prompt_len > 0) {
            char log_entry[CONSOLE_LINE_LEN];
            log_entry[0] = '>';
            log_entry[1] = ' ';
            size_t i = 0;
            for (; i < CONSOLE_LINE_LEN - 3 && prompt_buf[i] != '\0'; i++) {
                log_entry[2 + i] = prompt_buf[i];
            }
            log_entry[2 + i] = '\0';

            console_print(log_entry);

            /* Evaluate command in SerpentC runtime */
            serpentc_eval(prompt_buf);

            prompt_len = 0;
            prompt_buf[0] = '\0';
        }
    } else if (c >= 32 && c <= 126) {
        if (prompt_len + 1 < CONSOLE_PROMPT_MAX) {
            prompt_buf[prompt_len++] = c;
            prompt_buf[prompt_len] = '\0';
        }
    }
}

void console_draw(void) {
    if (!visible) return;

    uint32_t sw = vbe_get_width();
    uint32_t sh = vbe_get_height();

    uint32_t win_w = (sw > 40) ? (sw - 32) : sw;
    uint32_t win_h = 114;
    uint32_t win_x = 16;
    uint32_t win_y = (sh > win_h + 10) ? (sh - win_h - 10) : 0;

    /* Draw Console Outer Frame & Border */
    vbe_fill_rect(win_x, win_y, win_w, win_h, COLOR_AZTEC_GOLD);
    vbe_fill_rect(win_x + 2, win_y + 2, win_w - 4, win_h - 4, COLOR_OBSIDIAN);

    /* Header Bar */
    vbe_fill_rect(win_x + 2, win_y + 2, win_w - 4, 18, COLOR_JADE);
    vbe_draw_string(win_x + 8, win_y + 6, "[ SERPENT-C REPL CONSOLE ] (Toggle: ~)", COLOR_FEATHER_WHITE, COLOR_JADE);

    /* Render History Lines */
    for (size_t i = 0; i < history_count; i++) {
        vbe_draw_string(win_x + 8, win_y + 24 + (i * 12), history[i], COLOR_TURQUOISE, COLOR_OBSIDIAN);
    }

    /* Render Prompt Line */
    uint32_t prompt_y = win_y + win_h - 18;
    vbe_draw_string(win_x + 8, prompt_y, "> ", COLOR_JADE, COLOR_OBSIDIAN);
    vbe_draw_string(win_x + 24, prompt_y, prompt_buf, COLOR_FEATHER_WHITE, COLOR_OBSIDIAN);

    /* Animated Cursor */
    uint64_t ticks = pit_get_ticks();
    if ((ticks / 25) % 2 == 0) {
        vbe_draw_char(win_x + 24 + (prompt_len * 8), prompt_y, '_', COLOR_AZTEC_GOLD, COLOR_OBSIDIAN);
    }
}
