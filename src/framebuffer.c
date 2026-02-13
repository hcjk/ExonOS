#include "framebuffer.h"
#include "font8x8.h"

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void fb_put_pixel(const struct framebuffer *fb, int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || (uint32_t)x >= fb->width || (uint32_t)y >= fb->height) {
        return;
    }
    uint32_t *pixel = (uint32_t *)(fb->base + (uint32_t)y * fb->pitch + (uint32_t)x * 4);
    *pixel = color;
}

void fb_fill_rect(const struct framebuffer *fb, struct rect r, uint32_t color) {
    if (r.w <= 0 || r.h <= 0) {
        return;
    }
    int x0 = r.x < 0 ? 0 : r.x;
    int y0 = r.y < 0 ? 0 : r.y;
    int x1 = r.x + r.w;
    int y1 = r.y + r.h;
    if (x1 < 0 || y1 < 0) {
        return;
    }
    if (x1 > (int)fb->width) {
        x1 = (int)fb->width;
    }
    if (y1 > (int)fb->height) {
        y1 = (int)fb->height;
    }

    for (int y = y0; y < y1; ++y) {
        uint32_t *row = (uint32_t *)(fb->base + (uint32_t)y * fb->pitch + (uint32_t)x0 * 4);
        for (int x = x0; x < x1; ++x) {
            row[x - x0] = color;
        }
    }
}

void fb_draw_vertical_gradient(const struct framebuffer *fb, uint32_t top, uint32_t bottom) {
    for (uint32_t y = 0; y < fb->height; ++y) {
        uint32_t r = ((top >> 16) & 0xFF) + (((bottom >> 16) & 0xFF) - ((top >> 16) & 0xFF)) * y / (fb->height - 1);
        uint32_t g = ((top >> 8) & 0xFF) + (((bottom >> 8) & 0xFF) - ((top >> 8) & 0xFF)) * y / (fb->height - 1);
        uint32_t b = (top & 0xFF) + ((bottom & 0xFF) - (top & 0xFF)) * y / (fb->height - 1);
        uint32_t color = (r << 16) | (g << 8) | b;
        uint32_t *row = (uint32_t *)(fb->base + y * fb->pitch);
        for (uint32_t x = 0; x < fb->width; ++x) {
            row[x] = color;
        }
    }
}

void fb_draw_char(const struct framebuffer *fb, int x, int y, char c, uint32_t fg, uint32_t bg) {
    uint8_t glyph = (uint8_t)c;
    for (int row = 0; row < 8; ++row) {
        uint8_t bits = font8x8_basic[glyph][row];
        for (int col = 0; col < 8; ++col) {
            uint32_t color = (bits & (1u << (7 - col))) ? fg : bg;
            fb_put_pixel(fb, x + col, y + row, color);
        }
    }
}

void fb_draw_string(const struct framebuffer *fb, int x, int y, const char *text, uint32_t fg, uint32_t bg) {
    int cursor = 0;
    while (text[cursor] != '\0') {
        fb_draw_char(fb, x + cursor * 8, y, text[cursor], fg, bg);
        cursor++;
    }
}

int point_in_rect(int x, int y, struct rect r) {
    return x >= r.x && x < (r.x + r.w) && y >= r.y && y < (r.y + r.h);
}

void fb_blit(const struct framebuffer *dst, const struct framebuffer *src) {
    if (!dst || !src || !dst->base || !src->base) {
        return;
    }
    uint32_t rows = dst->height < src->height ? dst->height : src->height;
    uint32_t cols = dst->width < src->width ? dst->width : src->width;
    for (uint32_t y = 0; y < rows; ++y) {
        uint32_t *dst_row = (uint32_t *)(dst->base + y * dst->pitch);
        const uint32_t *src_row = (const uint32_t *)(src->base + y * src->pitch);
        uint32_t count = cols;
        __asm__ volatile ("rep movsd"
                          : "+D"(dst_row), "+S"(src_row), "+c"(count)
                          :
                          : "memory");
    }
}
