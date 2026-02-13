#include <stdint.h>

#include "magicui.h"
#include "framebuffer.h"

uint32_t mui_theme_color(int index) {
    switch (index) {
    case 0:
        return rgb(231, 125, 36);
    case 1:
        return rgb(54, 164, 98);
    case 2:
        return rgb(34, 82, 140);
    case 3:
        return rgb(196, 64, 64);
    case 4:
        return rgb(210, 210, 210);
    default:
        return rgb(34, 82, 140);
    }
}

const char *mui_theme_name(int index) {
    switch (index) {
    case 0:
        return "Orange";
    case 1:
        return "Green";
    case 2:
        return "Blue";
    case 3:
        return "Red";
    case 4:
        return "White";
    default:
        return "Blue";
    }
}

uint32_t mui_theme_background_top(int index) {
    switch (index) {
    case 0:
        return rgb(145, 78, 23);
    case 1:
        return rgb(39, 117, 70);
    case 2:
        return rgb(22, 54, 92);
    case 3:
        return rgb(156, 51, 51);
    case 4:
        return rgb(139, 139, 139);
    default:
        return rgb(22, 52, 88);
    }
}

uint32_t mui_theme_background_bottom(int index) {
    switch (index) {
    case 0:
        return rgb(145, 78, 23);
    case 1:
        return rgb(39, 117, 70);
    case 2:
        return rgb(22, 54, 92);
    case 3:
        return rgb(156, 51, 51);
    case 4:
        return rgb(139, 139, 139);
    default:
        return rgb(22, 52, 88);
    }
}

struct rect mui_window_titlebar_rect(struct rect r) {
    struct rect bar = { r.x + 2, r.y + 2, r.w - 4, 22 };
    return bar;
}

struct rect mui_window_close_rect(struct rect r) {
    struct rect close = { r.x + r.w - 22, r.y + 4, 16, 16 };
    return close;
}

void mui_draw_window(const struct framebuffer *fb, struct rect r, const char *title, uint32_t accent) {
    uint32_t border = rgb(15, 24, 42);
    uint32_t body = rgb(230, 234, 240);
    fb_fill_rect(fb, r, border);
    struct rect inner = { r.x + 2, r.y + 2, r.w - 4, r.h - 4 };
    fb_fill_rect(fb, inner, body);
    struct rect bar = mui_window_titlebar_rect(r);
    fb_fill_rect(fb, bar, accent);
    fb_draw_string(fb, r.x + 8, r.y + 6, title, rgb(255, 255, 255), accent);

    struct rect close = mui_window_close_rect(r);
    fb_fill_rect(fb, close, rgb(200, 64, 64));
    fb_draw_string(fb, close.x + 4, close.y + 3, "X", rgb(255, 255, 255), rgb(200, 64, 64));
}

void mui_draw_button(const struct framebuffer *fb, struct rect r, const char *label, uint32_t bg, uint32_t fg) {
    fb_fill_rect(fb, r, bg);
    fb_draw_string(fb, r.x + 8, r.y + 7, label, fg, bg);
}

void mui_draw_progress(const struct framebuffer *fb, struct rect r, uint32_t value, uint32_t max_value, uint32_t fill, uint32_t empty) {
    fb_fill_rect(fb, r, empty);
    if (max_value == 0) {
        return;
    }
    if (value > max_value) {
        value = max_value;
    }
    uint32_t width_u32 = (uint32_t)(r.w < 0 ? 0 : r.w);
    uint32_t fill_u32 = (width_u32 * value) / max_value;
    int fill_w = (int)fill_u32;
    if (fill_w < 0) {
        fill_w = 0;
    }
    if (fill_w > r.w) {
        fill_w = r.w;
    }
    struct rect fill_rect = { r.x, r.y, fill_w, r.h };
    fb_fill_rect(fb, fill_rect, fill);
}

void mui_draw_cursor(const struct framebuffer *fb, int x, int y) {
    uint32_t dark = rgb(30, 30, 30);
    uint32_t light = rgb(255, 255, 255);
    fb_put_pixel(fb, x, y, light);
    fb_put_pixel(fb, x + 1, y + 1, light);
    fb_put_pixel(fb, x + 2, y + 2, light);
    fb_put_pixel(fb, x + 3, y + 3, light);
    fb_put_pixel(fb, x + 1, y + 2, light);
    fb_put_pixel(fb, x + 2, y + 1, light);
    fb_put_pixel(fb, x + 4, y + 4, dark);
    fb_put_pixel(fb, x + 5, y + 5, dark);
    fb_put_pixel(fb, x + 5, y + 4, dark);
    fb_put_pixel(fb, x + 4, y + 5, dark);
}
