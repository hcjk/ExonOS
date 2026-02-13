#pragma once

#include "framebuffer.h"
#include "ui.h"

enum ui_app_id {
	UI_APP_APPS = 0,
	UI_APP_SETTINGS = 1,
	UI_APP_FILES = 2,
	UI_APP_USB = 3,
    UI_APP_TEST = 4,
	UI_APP_COUNT = 5
};

int ui_app_count(void);
const char *ui_app_menu_label(enum ui_app_id app_id);

int ui_app_has_desktop_icon(enum ui_app_id app_id);
struct rect ui_app_desktop_icon_rect(enum ui_app_id app_id);
const char *ui_app_desktop_icon_label(enum ui_app_id app_id);

int ui_app_is_open(const struct ui_state *state, enum ui_app_id app_id);
void ui_app_set_open(struct ui_state *state, enum ui_app_id app_id, int open);
struct rect *ui_app_rect_mut(struct ui_state *state, enum ui_app_id app_id);
struct rect ui_app_rect(const struct ui_state *state, enum ui_app_id app_id);

int ui_app_handle_click(struct ui_state *state, enum ui_app_id app_id, int mouse_x, int mouse_y);
void ui_app_render(enum ui_app_id app_id, const struct framebuffer *fb, const struct ui_state *state, uint32_t accent);

void app_apps_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent);
void app_files_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent);
void app_usb_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent);
void app_settings_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent);
void app_test_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent);
int app_settings_handle_click(struct ui_state *state, int mouse_x, int mouse_y);
