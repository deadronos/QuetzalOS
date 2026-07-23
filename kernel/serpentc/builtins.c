#include "builtins.h"
#include "../drivers/vbe.h"
#include "../drivers/pit.h"

static symbol_t symbols[SERPENTC_MAX_VARS];

static int str_equal(const char* s1, size_t len1, const char* s2) {
    size_t i = 0;
    for (; i < len1; i++) {
        if (s2[i] == '\0' || s1[i] != s2[i]) return 0;
    }
    return s2[i] == '\0';
}

void builtins_init(void) {
    for (size_t i = 0; i < SERPENTC_MAX_VARS; i++) {
        symbols[i].is_set = 0;
        symbols[i].name[0] = '\0';
        symbols[i].value = 0;
    }

    /* Predefine color constants in environment */
    symbol_set("COLOR_OBSIDIAN", 14, COLOR_OBSIDIAN);
    symbol_set("COLOR_JADE", 10, COLOR_JADE);
    symbol_set("COLOR_AZTEC_GOLD", 16, COLOR_AZTEC_GOLD);
    symbol_set("COLOR_COCHINEAL_RED", 19, COLOR_COCHINEAL_RED);
    symbol_set("COLOR_TURQUOISE", 15, COLOR_TURQUOISE);
    symbol_set("COLOR_FEATHER_WHITE", 19, COLOR_FEATHER_WHITE);
}

int symbol_set(const char* name, size_t len, uint32_t val) {
    if (len == 0 || len >= SERPENTC_MAX_NAME) return 0;

    /* Search for existing symbol */
    for (size_t i = 0; i < SERPENTC_MAX_VARS; i++) {
        if (symbols[i].is_set && str_equal(name, len, symbols[i].name)) {
            symbols[i].value = val;
            return 1;
        }
    }

    /* Find empty slot */
    for (size_t i = 0; i < SERPENTC_MAX_VARS; i++) {
        if (!symbols[i].is_set) {
            for (size_t j = 0; j < len; j++) {
                symbols[i].name[j] = name[j];
            }
            symbols[i].name[len] = '\0';
            symbols[i].value = val;
            symbols[i].is_set = 1;
            return 1;
        }
    }

    return 0; // Symbol table full
}

int symbol_get(const char* name, size_t len, uint32_t* out_val) {
    if (len == 0 || len >= SERPENTC_MAX_NAME) return 0;

    for (size_t i = 0; i < SERPENTC_MAX_VARS; i++) {
        if (symbols[i].is_set && str_equal(name, len, symbols[i].name)) {
            if (out_val) *out_val = symbols[i].value;
            return 1;
        }
    }
    return 0;
}

int dispatch_builtin(const char* name, size_t len, const uint32_t* args, size_t arg_count, uint32_t* out_result) {
    if (out_result) *out_result = 0;

    if (str_equal(name, len, "feather_draw_line")) {
        if (arg_count >= 5) {
            vbe_draw_line(args[0], args[1], args[2], args[3], args[4]);
            return 1;
        }
    } else if (str_equal(name, len, "feather_rect")) {
        if (arg_count >= 5) {
            vbe_fill_rect(args[0], args[1], args[2], args[3], args[4]);
            return 1;
        }
    } else if (str_equal(name, len, "feather_clear")) {
        if (arg_count >= 1) {
            vbe_clear(args[0]);
            return 1;
        }
    } else if (str_equal(name, len, "tonal_now")) {
        uint64_t ticks = pit_get_ticks();
        if (out_result) *out_result = (uint32_t)ticks;
        return 1;
    }

    return 0; // Unknown or wrong arg count
}
