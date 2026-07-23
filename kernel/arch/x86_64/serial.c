#include "serial.h"
#include "io.h"
#include <stdarg.h>

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static int is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_putc(char c) {
    while (is_transmit_empty() == 0);
    outb(COM1_PORT, c);
}

void serial_puts(const char* str) {
    if (!str) return;
    while (*str) {
        if (*str == '\n') {
            serial_putc('\r');
        }
        serial_putc(*str);
        str++;
    }
}

static void print_num(uint64_t val, int base, int is_signed) {
    char buf[32];
    int pos = 0;

    if (is_signed && (int64_t)val < 0) {
        serial_putc('-');
        val = (uint64_t)(-(int64_t)val);
    }

    if (val == 0) {
        serial_putc('0');
        return;
    }

    const char* digits = "0123456789ABCDEF";
    while (val > 0) {
        buf[pos++] = digits[val % (uint64_t)base];
        val /= (uint64_t)base;
    }

    while (pos > 0) {
        serial_putc(buf[--pos]);
    }
}

void kprintf(const char* fmt, ...) {
    if (!fmt) return;

    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == '\0') break;

            switch (*fmt) {
                case 's': {
                    const char* s = va_arg(args, const char*);
                    serial_puts(s ? s : "(null)");
                    break;
                }
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    print_num((uint64_t)(int64_t)val, 10, 1);
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    print_num((uint64_t)val, 10, 0);
                    break;
                }
                case 'x':
                case 'p': {
                    uint64_t val = va_arg(args, uint64_t);
                    if (*fmt == 'p') serial_puts("0x");
                    print_num(val, 16, 0);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    serial_putc(c);
                    break;
                }
                case '%': {
                    serial_putc('%');
                    break;
                }
                default: {
                    serial_putc('%');
                    serial_putc(*fmt);
                    break;
                }
            }
        } else {
            if (*fmt == '\n') {
                serial_putc('\r');
            }
            serial_putc(*fmt);
        }
        fmt++;
    }

    va_end(args);
}
