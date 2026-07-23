#ifndef QUETZAL_PIT_H
#define QUETZAL_PIT_H

#include <stdint.h>

// =============================================================================
// QuetzalOS PIT (8254) Driver + Tonalpohualli Calendar Engine
//
// The hardware timer is configured at pit_init(frequency_hz). pit_handler()
// must be installed as the IRQ0 vector (IDT entry 32).
//
// Thread-safety contract:
//   - `ticks` is updated inside pit_handler() (IRQ context) and read from any
//     privilege level. pit_get_ticks() uses __atomic_load_n with RELAXED
//     ordering so the compiler is forbidden from emitting a torn 64-bit read
//     and so future SMP work does not need to touch call sites again.
//   - pit_get_tonal_time() returns a struct by value; the embedded sign_name
//     pointer aliases a static array and is safe to read after the call.
// =============================================================================

typedef struct {
    uint32_t trecena_num;      // 1 to 13
    uint32_t veintena_sign;    // 0 to 19 (Day Sign Index)
    const char* sign_name;     // Name of the day sign (static lifetime)
    uint32_t total_tonal_days; // Days elapsed in Tonalpohualli cycle
    uint64_t total_ticks;      // Total hardware timer ticks
} tonalpohualli_time_t;

void          pit_init(uint32_t frequency_hz);
void          pit_handler(void);
uint64_t      pit_get_ticks(void);
tonalpohualli_time_t pit_get_tonal_time(void);

#endif /* QUETZAL_PIT_H */
