#include "log.h"
#include "portio.h"

#define COM1 0x3F8

static int log_ready(void) {
    return (inb(COM1 + 5) & 0x20) != 0;
}

void log_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

void log_putc(char c) {
    for (uint32_t i = 0; i < 100000; ++i) {
        if (log_ready()) {
            outb(COM1, (uint8_t)c);
            return;
        }
    }
}

void log_write(const char *s, uint32_t len) {
    if (!s) {
        return;
    }
    for (uint32_t i = 0; i < len; ++i) {
        log_putc(s[i]);
    }
}

void log_puts(const char *s) {
    if (!s) {
        return;
    }
    while (*s) {
        log_putc(*s++);
    }
}

void log_hex32(uint32_t value) {
    const char *hex = "0123456789ABCDEF";
    log_puts("0x");
    for (int i = 7; i >= 0; --i) {
        uint8_t nibble = (uint8_t)((value >> (i * 4)) & 0xF);
        log_putc(hex[nibble]);
    }
}

void log_dec32(uint32_t value) {
    char buf[11];
    int i = 0;
    if (value == 0) {
        log_putc('0');
        return;
    }
    while (value > 0 && i < (int)(sizeof(buf) - 1)) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (i > 0) {
        log_putc(buf[--i]);
    }
}
