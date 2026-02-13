#include <stdint.h>

#include "ui_apps.h"

static const char *k_menu_labels[UI_APP_COUNT] = {
    "Apps",
    "Settings",
    "Folders",
    "USB Manager",
    "Test App"
};

static const int k_has_desktop_icon[UI_APP_COUNT] = {
    0,
    0,
    0,
    1,
    1
};

static const struct rect k_desktop_icon_rects[UI_APP_COUNT] = {
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 24, 60, 64, 64 },
    { 24, 132, 64, 64 }
};

static const char *k_desktop_icon_labels[UI_APP_COUNT] = {
    "",
    "",
    "",
    "USB Manager",
    "Test App"
};

int ui_app_count(void) {
    return UI_APP_COUNT;
}

const char *ui_app_menu_label(enum ui_app_id app_id) {
    if (app_id < 0 || app_id >= UI_APP_COUNT) {
        return "Unknown";
    }
    return k_menu_labels[app_id];
}

int ui_app_has_desktop_icon(enum ui_app_id app_id) {
    if (app_id < 0 || app_id >= UI_APP_COUNT) {
        return 0;
    }
    return k_has_desktop_icon[app_id];
}

struct rect ui_app_desktop_icon_rect(enum ui_app_id app_id) {
    if (app_id < 0 || app_id >= UI_APP_COUNT) {
        return (struct rect){ 0, 0, 0, 0 };
    }
    return k_desktop_icon_rects[app_id];
}

const char *ui_app_desktop_icon_label(enum ui_app_id app_id) {
    if (app_id < 0 || app_id >= UI_APP_COUNT) {
        return "";
    }
    return k_desktop_icon_labels[app_id];
}

int ui_app_is_open(const struct ui_state *state, enum ui_app_id app_id) {
    if (!state) {
        return 0;
    }
    switch (app_id) {
    case UI_APP_APPS:
        return state->apps_open;
    case UI_APP_SETTINGS:
        return state->settings_open;
    case UI_APP_FILES:
        return state->files_open;
    case UI_APP_USB:
        return state->usb_open;
    case UI_APP_TEST:
        return state->test_open;
    default:
        return 0;
    }
}

void ui_app_set_open(struct ui_state *state, enum ui_app_id app_id, int open) {
    if (!state) {
        return;
    }
    int value = open ? 1 : 0;
    switch (app_id) {
    case UI_APP_APPS:
        state->apps_open = value;
        break;
    case UI_APP_SETTINGS:
        state->settings_open = value;
        break;
    case UI_APP_FILES:
        state->files_open = value;
        break;
    case UI_APP_USB:
        state->usb_open = value;
        break;
    case UI_APP_TEST:
        state->test_open = value;
        break;
    default:
        break;
    }
}

struct rect *ui_app_rect_mut(struct ui_state *state, enum ui_app_id app_id) {
    if (!state) {
        return 0;
    }
    switch (app_id) {
    case UI_APP_APPS:
        return &state->apps_rect;
    case UI_APP_SETTINGS:
        return &state->settings_rect;
    case UI_APP_FILES:
        return &state->files_rect;
    case UI_APP_USB:
        return &state->usb_rect;
    case UI_APP_TEST:
        return &state->test_rect;
    default:
        return 0;
    }
}

struct rect ui_app_rect(const struct ui_state *state, enum ui_app_id app_id) {
    if (!state) {
        return (struct rect){ 0, 0, 0, 0 };
    }
    switch (app_id) {
    case UI_APP_APPS:
        return state->apps_rect;
    case UI_APP_SETTINGS:
        return state->settings_rect;
    case UI_APP_FILES:
        return state->files_rect;
    case UI_APP_USB:
        return state->usb_rect;
    case UI_APP_TEST:
        return state->test_rect;
    default:
        return (struct rect){ 0, 0, 0, 0 };
    }
}

int ui_app_handle_click(struct ui_state *state, enum ui_app_id app_id, int mouse_x, int mouse_y) {
    switch (app_id) {
    case UI_APP_SETTINGS:
        return app_settings_handle_click(state, mouse_x, mouse_y);
    default:
        return 0;
    }
}

void ui_app_render(enum ui_app_id app_id, const struct framebuffer *fb, const struct ui_state *state, uint32_t accent) {
    switch (app_id) {
    case UI_APP_APPS:
        app_apps_render(fb, state, accent);
        break;
    case UI_APP_SETTINGS:
        app_settings_render(fb, state, accent);
        break;
    case UI_APP_FILES:
        app_files_render(fb, state, accent);
        break;
    case UI_APP_USB:
        app_usb_render(fb, state, accent);
        break;
    case UI_APP_TEST:
        app_test_render(fb, state, accent);
        break;
    default:
        break;
    }
}
