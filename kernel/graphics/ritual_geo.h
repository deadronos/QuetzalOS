#ifndef QUETZAL_RITUAL_GEO_H
#define QUETZAL_RITUAL_GEO_H

#include <stdint.h>

void draw_step_pyramid(uint32_t cx, uint32_t cy, uint32_t base_width, uint32_t steps);
void draw_feathered_serpent(uint32_t x, uint32_t y, uint32_t length, uint64_t tick);
void draw_sun_disk(uint32_t cx, uint32_t cy, uint32_t radius, uint64_t tick);

#endif /* QUETZAL_RITUAL_GEO_H */
