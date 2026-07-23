#ifndef QUETZAL_PS2_KEYBOARD_H
#define QUETZAL_PS2_KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
void keyboard_handler(void);
char keyboard_getchar(void);
int  keyboard_has_char(void);

#endif /* QUETZAL_PS2_KEYBOARD_H */
