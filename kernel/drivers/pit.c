#include "pit.h"
#include "../arch/x86_64/io.h"

#include <stdatomic.h>

static _Atomic uint64_t ticks = 0;

static const char* DAY_SIGNS[20] = {
    "Cipactli (Crocodile)", "Ehecatl (Wind)", "Calli (House)", "Cuetzpalin (Lizard)",
    "Coatl (Serpent)", "Miquiztli (Death)", "Mazatl (Deer)", "Tochtli (Rabbit)",
    "Atl (Water)", "Itzcuintli (Dog)", "Ozomatli (Monkey)", "Malinalli (Grass)",
    "Acatl (Reed)", "Ocelotl (Jaguar)", "Cuauhtli (Eagle)", "Cozcacuauhtli (Vulture)",
    "Ollin (Movement)", "Tecpatl (Flint)", "Quiahuitl (Rain)", "Xochitl (Flower)"
};

void pit_init(uint32_t frequency_hz) {
    uint32_t divisor = 1193182u / frequency_hz;

    /* Channel 0, lobyte/hibyte, Mode 3 (Square wave generator) */
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFFu));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFFu));
}

void pit_handler(void) {
    /* RELAXED is sufficient: the caller does not need any happens-before
     * relationship with other memory, only the monotonic count. */
    atomic_fetch_add_explicit(&ticks, 1u, memory_order_relaxed);
    /* Send EOI to PIC Master */
    outb(0x20, 0x20);
}

uint64_t pit_get_ticks(void) {
    return atomic_load_explicit(&ticks, memory_order_relaxed);
}

tonalpohualli_time_t pit_get_tonal_time(void) {
    /* 1 day in ritual calendar = 100 ticks (1 second) for OS demonstration */
    uint64_t now = pit_get_ticks();
    uint32_t total_days = (uint32_t)((now / 100u) % 260u);

    tonalpohualli_time_t t;
    t.trecena_num      = (total_days % 13u) + 1u;
    t.veintena_sign    = total_days % 20u;
    t.sign_name        = DAY_SIGNS[t.veintena_sign];
    t.total_tonal_days = total_days;
    t.total_ticks      = now;
    return t;
}
