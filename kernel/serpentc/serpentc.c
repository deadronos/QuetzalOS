#include "serpentc.h"
#include "../drivers/vbe.h"
#include "../drivers/pit.h"

void serpentc_init(void) {
    /* Initialize SerpentC JIT Heap Arena at 0x01000000 */
    uint32_t* serpent_arena = (uint32_t*)0x01000000;
    serpent_arena[0] = 0x53455250; // "SERP" Magic marker
}

void serpentc_eval(const char* script) {
    /* Lightweight demonstration evaluator executing built-in commands */
    if (!script) return;

    vbe_draw_string(16, 560, "SERPENT-C > EVAL:", COLOR_TURQUOISE, COLOR_OBSIDIAN);
    vbe_draw_string(168, 560, script, COLOR_FEATHER_WHITE, COLOR_OBSIDIAN);
}
