#pragma once

#include <stdint.h>

struct ui_state;
struct framebuffer;

enum key_action {
    KEY_NONE,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_ESC,
    KEY_TAB,
    KEY_START
};

void init_ps2_mouse(void);
enum key_action poll_keyboard(void);
void poll_mouse(struct ui_state *state, const struct framebuffer *fb);
void input_inject_key(enum key_action action);
void input_inject_mouse(int dx, int dy, uint8_t buttons);
