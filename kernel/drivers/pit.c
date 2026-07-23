#include "pit.h"
#include "../arch/x86_64/io.h"

static volatile uint64_t ticks = 0;

static const char* DAY_SIGNS[20] = {
    "Cipactli (Crocodile)", "Ehecatl (Wind)", "Calli (House)", "Cuetzpalin (Lizard)",
    "Coatl (Serpent)", "Miquiztli (Death)", "Mazatl (Deer)", "Tochtli (Rabbit)",
    "Atl (Water)", "Itzcuintli (Dog)", "Ozomatli (Monkey)", "Malinalli (Grass)",
    "Acatl (Reed)", "Ocelotl (Jaguar)", "Cuauhtli (Eagle)", "Cozcacuauhtli (Vulture)",
    "Ollin (Movement)", "Tecpatl (Flint)", "Quiahuitl (Rain)", "Xochitl (Flower)"
};

void pit_init(uint32_t frequency_hz) {
    uint32_t divisor = 1193182 / frequency_hz;

    /* Channel 0, lobyte/hibyte, Mode 3 (Square wave generator) */
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void pit_handler(void) {
    ticks++;
    /* Send EOI to PIC Master */
    outb(0x20, 0x20);
}

uint64_t pit_get_ticks(void) {
    return ticks;
}

tonalpohualli_time_t pit_get_tonal_time(void) {
    /* 1 day in ritual calendar = 100 ticks (1 second) for OS demonstration */
    uint32_t total_days = (ticks / 100) % 260;

    tonalpohualli_time_t t;
    t.trecena_num = (total_days % 13) + 1;
    t.veintena_sign = total_days % 20;
    t.sign_name = DAY_SIGNS[t.veintena_sign];
    t.total_tonal_days = total_days;
    t.total_ticks = ticks;
    return t;
}
