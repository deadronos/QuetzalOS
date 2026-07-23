#ifndef QUETZAL_PIT_H
#define QUETZAL_PIT_H

#include <stdint.h>

typedef struct {
    uint32_t trecena_num;      // 1 to 13
    uint32_t veintena_sign;    // 0 to 19 (Day Sign Index)
    const char* sign_name;     // Name of the day sign
    uint32_t total_tonal_days; // Days elapsed in Tonalpohualli cycle
    uint64_t total_ticks;      // Total hardware timer ticks
} tonalpohualli_time_t;

void pit_init(uint32_t frequency_hz);
void pit_handler(void);
tonalpohualli_time_t pit_get_tonal_time(void);
uint64_t pit_get_ticks(void);

#endif /* QUETZAL_PIT_H */
