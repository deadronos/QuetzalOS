#ifndef QUETZAL_PS2_MOUSE_H
#define QUETZAL_PS2_MOUSE_H

#include <stdint.h>

void mouse_init(void);
void mouse_handler(void);
void mouse_get_position(uint32_t* out_x, uint32_t* out_y);
uint8_t mouse_get_buttons(void);
void mouse_draw_cursor(void);

#endif /* QUETZAL_PS2_MOUSE_H */
