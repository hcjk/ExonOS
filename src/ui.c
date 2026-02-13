#include <stdint.h>

#include "ui.h"
#include "framebuffer.h"
#include "input.h"
#include "magicui.h"
#include "ui_apps.h"

static void handle_menu_action(struct ui_state *state, int index) {
    state->menu_open = 0;
    if (index >= 0 && index < ui_app_count()) {
        ui_app_set_open(state, (enum ui_app_id)index, 1);
    }
}

static struct rect *drag_rect(struct ui_state *state) {
    if (!state) {
        return 0;
    }
    if (state->drag_app_id < 0 || state->drag_app_id >= ui_app_count()) {
        return 0;
    }
    return ui_app_rect_mut(state, (enum ui_app_id)state->drag_app_id);
}

static struct rect menu_item_rect(struct rect panel, int index) {
    struct rect item = { panel.x + 8, panel.y + 10 + index * 42, 164, 28 };
    return item;
}

static void clamp_rect(struct rect *r, const struct framebuffer *fb) {
    if (!r || !fb) {
        return;
    }
    int max_x = (int)fb->width - r->w;
    int max_y = (int)fb->height - r->h - 32;
    if (max_x < 0) {
        max_x = 0;
    }
    if (max_y < 0) {
        max_y = 0;
    }
    if (r->x < 0) {
        r->x = 0;
    }
    if (r->y < 0) {
        r->y = 0;
    }
    if (r->x > max_x) {
        r->x = max_x;
    }
    if (r->y > max_y) {
        r->y = max_y;
    }
}

void ui_init(struct ui_state *state, const struct framebuffer *fb, const struct system_info *info) {
    state->menu_open = 1;
    state->menu_index = 0;
    state->apps_open = 0;
    state->settings_open = 0;
    state->files_open = 0;
    state->usb_open = 0;
    state->test_open = 0;
    state->mouse_buttons = 0;
    state->prev_mouse_buttons = 0;
    state->mouse_x = (int)fb->width / 2;
    state->mouse_y = (int)fb->height / 2;
    state->theme_index = 2;
    state->drag_app_id = -1;
    state->drag_offset_x = 0;
    state->drag_offset_y = 0;
    state->apps_rect = (struct rect){ 90, 90, 320, 200 };
    state->settings_rect = (struct rect){ 150, 120, 420, 260 };
    state->files_rect = (struct rect){ 220, 100, 320, 220 };
    state->usb_rect = (struct rect){ 260, 160, 360, 220 };
    state->test_rect = (struct rect){ 300, 120, 340, 200 };
    if (info) {
        state->info = *info;
    } else {
        state->info.version[0] = '\0';
        state->info.kernel[0] = '\0';
        state->info.cpu[0] = '\0';
        state->info.ram_kb = 0;
        state->info.width = fb->width;
        state->info.height = fb->height;
        state->info.bpp = fb->bpp;
    }
}

void ui_update(struct ui_state *state, const struct framebuffer *fb) {
    int app_count = ui_app_count();
    enum key_action key = poll_keyboard();
    poll_mouse(state, fb);

    if (key == KEY_START) {
        state->menu_open = !state->menu_open;
    } else if (key == KEY_ESC) {
        state->menu_open = 0;
    }

    if (state->menu_open) {
        if (key == KEY_UP) {
            state->menu_index = (state->menu_index + app_count - 1) % app_count;
        } else if (key == KEY_DOWN || key == KEY_TAB) {
            state->menu_index = (state->menu_index + 1) % app_count;
        } else if (key == KEY_ENTER) {
            handle_menu_action(state, state->menu_index);
        }
    }

    int left_down = (state->mouse_buttons & 1) != 0;
    int left_press = left_down && !(state->prev_mouse_buttons & 1);
    int left_release = !left_down && (state->prev_mouse_buttons & 1);
    state->prev_mouse_buttons = state->mouse_buttons;

    struct rect taskbar = { 0, (int)fb->height - 32, (int)fb->width, 32 };
    struct rect start_btn = { 8, taskbar.y + 4, 72, 24 };
    struct rect menu_panel = { 8, taskbar.y - (app_count * 42 + 12), 180, app_count * 42 + 12 };

    if (left_press) {
        int handled_click = 0;
        if (point_in_rect(state->mouse_x, state->mouse_y, start_btn)) {
            state->menu_open = !state->menu_open;
            handled_click = 1;
        }

        if (!handled_click && state->menu_open) {
            for (int i = 0; i < app_count; ++i) {
                struct rect item = menu_item_rect(menu_panel, i);
                if (point_in_rect(state->mouse_x, state->mouse_y, item)) {
                    handle_menu_action(state, i);
                    handled_click = 1;
                    break;
                }
            }
        }

        if (!handled_click) {
            for (int i = 0; i < app_count; ++i) {
                enum ui_app_id app_id = (enum ui_app_id)i;
                if (!ui_app_has_desktop_icon(app_id)) {
                    continue;
                }
                struct rect icon = ui_app_desktop_icon_rect(app_id);
                if (point_in_rect(state->mouse_x, state->mouse_y, icon)) {
                    ui_app_set_open(state, app_id, 1);
                    handled_click = 1;
                    break;
                }
            }
        }

        if (!handled_click) {
            for (int i = app_count - 1; i >= 0; --i) {
                enum ui_app_id app_id = (enum ui_app_id)i;
                if (!ui_app_is_open(state, app_id)) {
                    continue;
                }
                struct rect rect = ui_app_rect(state, app_id);
                if (point_in_rect(state->mouse_x, state->mouse_y, mui_window_close_rect(rect))) {
                    ui_app_set_open(state, app_id, 0);
                    handled_click = 1;
                    break;
                }
            }
        }

        if (!handled_click) {
            for (int i = app_count - 1; i >= 0; --i) {
                enum ui_app_id app_id = (enum ui_app_id)i;
                if (!ui_app_is_open(state, app_id)) {
                    continue;
                }
                if (ui_app_handle_click(state, app_id, state->mouse_x, state->mouse_y)) {
                    handled_click = 1;
                    break;
                }
            }
        }

        if (!handled_click) {
            for (int i = app_count - 1; i >= 0; --i) {
                enum ui_app_id app_id = (enum ui_app_id)i;
                if (!ui_app_is_open(state, app_id)) {
                    continue;
                }
                struct rect rect = ui_app_rect(state, app_id);
                if (point_in_rect(state->mouse_x, state->mouse_y, mui_window_titlebar_rect(rect))) {
                    state->drag_app_id = i;
                    state->drag_offset_x = state->mouse_x - rect.x;
                    state->drag_offset_y = state->mouse_y - rect.y;
                    break;
                }
            }
        }
    }

    if (left_down && state->drag_app_id >= 0) {
        struct rect *r = drag_rect(state);
        if (r) {
            r->x = state->mouse_x - state->drag_offset_x;
            r->y = state->mouse_y - state->drag_offset_y;
            clamp_rect(r, fb);
        }
    }

    if (left_release) {
        state->drag_app_id = -1;
    }
}

void ui_render(const struct framebuffer *fb, const struct ui_state *state) {
    uint32_t accent = mui_theme_color(state->theme_index);
    uint32_t top = mui_theme_background_top(state->theme_index);
    uint32_t bottom = mui_theme_background_bottom(state->theme_index);
    fb_draw_vertical_gradient(fb, top, bottom);

    struct rect taskbar = { 0, (int)fb->height - 32, (int)fb->width, 32 };
    fb_fill_rect(fb, taskbar, rgb(15, 24, 42));

    struct rect start_btn = { 8, taskbar.y + 4, 72, 24 };
    mui_draw_button(fb, start_btn, "Start", accent, rgb(255, 255, 255));

    fb_draw_string(fb, 100, taskbar.y + 10, "ExonOS Prototype", rgb(200, 210, 230), rgb(15, 24, 42));

    int app_count = ui_app_count();
    for (int i = 0; i < app_count; ++i) {
        enum ui_app_id app_id = (enum ui_app_id)i;
        if (!ui_app_has_desktop_icon(app_id)) {
            continue;
        }
        struct rect icon = ui_app_desktop_icon_rect(app_id);
        mui_draw_button(fb, icon, "USB", rgb(30, 40, 60), rgb(240, 240, 240));
        fb_draw_string(fb, icon.x - 4, icon.y + 70, ui_app_desktop_icon_label(app_id), rgb(230, 230, 230), rgb(0, 0, 0));
    }

    if (state->menu_open) {
        struct rect panel = { 8, taskbar.y - (app_count * 42 + 12), 180, app_count * 42 + 12 };
        fb_fill_rect(fb, panel, rgb(30, 40, 60));

        for (int i = 0; i < app_count; ++i) {
            struct rect item = menu_item_rect(panel, i);
            mui_draw_button(fb, item, ui_app_menu_label((enum ui_app_id)i), state->menu_index == i ? accent : rgb(40, 52, 72), rgb(240, 240, 240));
        }
    }

    for (int i = 0; i < app_count; ++i) {
        enum ui_app_id app_id = (enum ui_app_id)i;
        if (ui_app_is_open(state, app_id)) {
            ui_app_render(app_id, fb, state, accent);
        }
    }

    mui_draw_cursor(fb, state->mouse_x, state->mouse_y);
}
