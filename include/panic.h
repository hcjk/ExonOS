#pragma once

struct framebuffer;

void panic_set_framebuffer(const struct framebuffer *fb);
void panic(const char *msg) __attribute__((noreturn));
