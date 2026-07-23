#ifndef QOS_SERIAL_H
#define QOS_SERIAL_H

#include <stdint.h>

#define COM1_PORT 0x3F8

void serial_init(void);
void serial_putc(char c);
void serial_puts(const char* str);
void kprintf(const char* fmt, ...);

#endif /* QOS_SERIAL_H */
