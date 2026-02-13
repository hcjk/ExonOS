#pragma once

#include <stdint.h>

struct framebuffer {
    uint8_t *base;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
};

struct rect {
    int x;
    int y;
    int w;
    int h;
};

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);
void fb_put_pixel(const struct framebuffer *fb, int x, int y, uint32_t color);
void fb_fill_rect(const struct framebuffer *fb, struct rect r, uint32_t color);
void fb_draw_vertical_gradient(const struct framebuffer *fb, uint32_t top, uint32_t bottom);
void fb_draw_char(const struct framebuffer *fb, int x, int y, char c, uint32_t fg, uint32_t bg);
void fb_draw_string(const struct framebuffer *fb, int x, int y, const char *text, uint32_t fg, uint32_t bg);
int point_in_rect(int x, int y, struct rect r);
void fb_blit(const struct framebuffer *dst, const struct framebuffer *src);
