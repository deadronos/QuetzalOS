#ifndef QUETZAL_CONSOLE_H
#define QUETZAL_CONSOLE_H

#include <stdint.h>
#include <stddef.h>

#define CONSOLE_MAX_HISTORY 5
#define CONSOLE_LINE_LEN    64
#define CONSOLE_PROMPT_MAX  128

void console_init(void);
void console_toggle(void);
int  console_is_visible(void);
void console_handle_char(char c);
void console_print(const char* msg);
void console_draw(void);

#endif /* QUETZAL_CONSOLE_H */
