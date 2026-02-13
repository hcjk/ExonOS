#pragma once

#include <stdint.h>

void log_init(void);
void log_putc(char c);
void log_puts(const char *s);
void log_write(const char *s, uint32_t len);
void log_hex32(uint32_t value);
void log_dec32(uint32_t value);
