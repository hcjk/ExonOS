#pragma once

#include <stdint.h>
#include "framebuffer.h"

uint32_t mui_theme_color(int index);
const char *mui_theme_name(int index);
uint32_t mui_theme_background_top(int index);
uint32_t mui_theme_background_bottom(int index);

struct rect mui_window_titlebar_rect(struct rect r);
struct rect mui_window_close_rect(struct rect r);

void mui_draw_window(const struct framebuffer *fb, struct rect r, const char *title, uint32_t accent);
void mui_draw_button(const struct framebuffer *fb, struct rect r, const char *label, uint32_t bg, uint32_t fg);
void mui_draw_progress(const struct framebuffer *fb, struct rect r, uint32_t value, uint32_t max_value, uint32_t fill, uint32_t empty);
void mui_draw_cursor(const struct framebuffer *fb, int x, int y);
