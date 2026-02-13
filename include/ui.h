#pragma once

#include <stdint.h>
#include "framebuffer.h"

#define SYSINFO_STR_LEN 64

struct system_info {
    char version[16];
    char kernel[32];
    char cpu[SYSINFO_STR_LEN];
    uint32_t ram_kb;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
};

struct ui_state {
    int menu_open;
    int menu_index;
    int apps_open;
    int settings_open;
    int files_open;
    int usb_open;
    int test_open;
    int mouse_x;
    int mouse_y;
    uint8_t mouse_buttons;
    uint8_t prev_mouse_buttons;
    int theme_index;
    int drag_app_id;
    int drag_offset_x;
    int drag_offset_y;
    struct rect apps_rect;
    struct rect settings_rect;
    struct rect files_rect;
    struct rect usb_rect;
    struct rect test_rect;
    struct system_info info;
};

void ui_init(struct ui_state *state, const struct framebuffer *fb, const struct system_info *info);
void ui_update(struct ui_state *state, const struct framebuffer *fb);
void ui_render(const struct framebuffer *fb, const struct ui_state *state);
