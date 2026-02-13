#include <stdint.h>

#include "ui_apps.h"
#include "magicui.h"
#include "framebuffer.h"

static uint32_t str_len(const char *s) {
    uint32_t len = 0;
    if (!s) {
        return 0;
    }
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

static void str_copy(char *dst, const char *src, uint32_t max_len) {
    if (!dst || !src || max_len == 0) {
        return;
    }
    uint32_t i = 0;
    while (i + 1 < max_len && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static void str_append(char *dst, const char *src, uint32_t max_len) {
    if (!dst || !src || max_len == 0) {
        return;
    }
    uint32_t len = str_len(dst);
    uint32_t i = 0;
    while (len + 1 < max_len && src[i] != '\0') {
        dst[len++] = src[i++];
    }
    dst[len] = '\0';
}

static void u32_to_dec(uint32_t value, char *out, uint32_t max_len) {
    if (!out || max_len == 0) {
        return;
    }
    char tmp[16];
    uint32_t i = 0;
    if (value == 0) {
        tmp[i++] = '0';
    } else {
        while (value > 0 && i < sizeof(tmp)) {
            tmp[i++] = (char)('0' + (value % 10));
            value /= 10;
        }
    }

    uint32_t out_i = 0;
    while (i > 0 && out_i + 1 < max_len) {
        out[out_i++] = tmp[--i];
    }
    out[out_i] = '\0';
}

static void format_ram(uint32_t ram_kb, char *out, uint32_t max_len) {
    if (ram_kb == 0) {
        str_copy(out, "Unknown", max_len);
        return;
    }
    uint32_t ram_mb = ram_kb / 1024;
    char num[16];
    u32_to_dec(ram_mb, num, sizeof(num));
    out[0] = '\0';
    str_append(out, num, max_len);
    str_append(out, " MB", max_len);
}

static void format_resolution(uint32_t w, uint32_t h, char *out, uint32_t max_len) {
    char num[16];
    out[0] = '\0';
    u32_to_dec(w, num, sizeof(num));
    str_append(out, num, max_len);
    str_append(out, "x", max_len);
    u32_to_dec(h, num, sizeof(num));
    str_append(out, num, max_len);
}

static void format_bpp(uint8_t bpp, char *out, uint32_t max_len) {
    char num[8];
    u32_to_dec(bpp, num, sizeof(num));
    out[0] = '\0';
    str_append(out, num, max_len);
    str_append(out, " bpp", max_len);
}

int app_settings_handle_click(struct ui_state *state, int mouse_x, int mouse_y) {
    struct rect panel = state->settings_rect;
    int y = panel.y + 44;
    int x = panel.x + 16;
    int spacing = 34;

    for (int i = 0; i < 5; ++i) {
        struct rect button = { x + i * spacing, y, 24, 24 };
        if (point_in_rect(mouse_x, mouse_y, button)) {
            state->theme_index = i;
            return 1;
        }
    }

    return 0;
}

void app_settings_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent) {
    mui_draw_window(fb, state->settings_rect, "Settings", accent);

    int content_x = state->settings_rect.x + 16;
    int content_y = state->settings_rect.y + 36;
    fb_draw_string(fb, content_x, content_y, "Theme Colors", rgb(60, 60, 60), rgb(230, 234, 240));

    int btn_y = content_y + 14;
    int btn_x = content_x;
    int spacing = 34;

    for (int i = 0; i < 5; ++i) {
        struct rect button = { btn_x + i * spacing, btn_y, 24, 24 };
        fb_fill_rect(fb, button, mui_theme_color(i));
        if (state->theme_index == i) {
            struct rect outline = { button.x - 2, button.y - 2, button.w + 4, button.h + 4 };
            fb_fill_rect(fb, outline, rgb(20, 20, 20));
            fb_fill_rect(fb, button, mui_theme_color(i));
        }
    }

    int labels_y = btn_y + 30;
    fb_draw_string(fb, content_x, labels_y, mui_theme_name(state->theme_index), rgb(60, 60, 60), rgb(230, 234, 240));

    int info_y = labels_y + 24;
    char buffer[64];
    char line[64];

    str_copy(buffer, "OS: ", sizeof(buffer));
    str_append(buffer, state->info.version, sizeof(buffer));
    fb_draw_string(fb, content_x, info_y, buffer, rgb(60, 60, 60), rgb(230, 234, 240));
    info_y += 16;

    str_copy(buffer, "Kernel: ", sizeof(buffer));
    str_append(buffer, state->info.kernel, sizeof(buffer));
    fb_draw_string(fb, content_x, info_y, buffer, rgb(60, 60, 60), rgb(230, 234, 240));
    info_y += 16;

    str_copy(buffer, "CPU: ", sizeof(buffer));
    str_append(buffer, state->info.cpu[0] ? state->info.cpu : "Unknown", sizeof(buffer));
    fb_draw_string(fb, content_x, info_y, buffer, rgb(60, 60, 60), rgb(230, 234, 240));
    info_y += 16;

    format_ram(state->info.ram_kb, buffer, sizeof(buffer));
    str_copy(line, "RAM: ", sizeof(line));
    str_append(line, buffer, sizeof(line));
    fb_draw_string(fb, content_x, info_y, line, rgb(60, 60, 60), rgb(230, 234, 240));
    info_y += 16;

    format_resolution(state->info.width, state->info.height, buffer, sizeof(buffer));
    str_copy(line, "Resolution: ", sizeof(line));
    str_append(line, buffer, sizeof(line));
    fb_draw_string(fb, content_x, info_y, line, rgb(60, 60, 60), rgb(230, 234, 240));
    info_y += 16;

    format_bpp(state->info.bpp, buffer, sizeof(buffer));
    str_copy(line, "Color Depth: ", sizeof(line));
    str_append(line, buffer, sizeof(line));
    fb_draw_string(fb, content_x, info_y, line, rgb(60, 60, 60), rgb(230, 234, 240));

    struct rect progress = { state->settings_rect.x + 16, state->settings_rect.y + state->settings_rect.h - 24, state->settings_rect.w - 32, 10 };
    mui_draw_progress(fb, progress, (uint32_t)(state->theme_index + 1), 5, accent, rgb(180, 185, 195));
}
