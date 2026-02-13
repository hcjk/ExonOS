#include "input.h"
#include "framebuffer.h"
#include "portio.h"
#include "ui.h"

#define KEY_QUEUE_SIZE 16

static enum key_action key_queue[KEY_QUEUE_SIZE];
static uint8_t key_head;
static uint8_t key_tail;
static int pending_mouse_dx;
static int pending_mouse_dy;
static uint8_t pending_mouse_buttons;
static uint8_t pending_mouse_valid;

void input_inject_key(enum key_action action) {
    uint8_t next = (uint8_t)((key_tail + 1) % KEY_QUEUE_SIZE);
    if (next == key_head) {
        return;
    }
    key_queue[key_tail] = action;
    key_tail = next;
}

void input_inject_mouse(int dx, int dy, uint8_t buttons) {
    pending_mouse_dx += dx;
    pending_mouse_dy += dy;
    pending_mouse_buttons = buttons;
    pending_mouse_valid = 1;
}

static void ps2_wait_read(void) {
    while ((inb(0x64) & 0x01) == 0) {
    }
}

static void ps2_wait_write(void) {
    while ((inb(0x64) & 0x02) != 0) {
    }
}

static void ps2_write_cmd(uint8_t cmd) {
    ps2_wait_write();
    outb(0x64, cmd);
}

static void ps2_write_data(uint8_t data) {
    ps2_wait_write();
    outb(0x60, data);
}

static uint8_t ps2_read_data(void) {
    ps2_wait_read();
    return inb(0x60);
}

static void ps2_mouse_write(uint8_t data) {
    ps2_write_cmd(0xD4);
    ps2_write_data(data);
}

void init_ps2_mouse(void) {
    ps2_write_cmd(0xA8);

    ps2_write_cmd(0x20);
    uint8_t status = ps2_read_data();
    status |= 0x02;
    ps2_write_cmd(0x60);
    ps2_write_data(status);

    ps2_mouse_write(0xF6);
    (void)ps2_read_data();
    ps2_mouse_write(0xF4);
    (void)ps2_read_data();
}

enum key_action poll_keyboard(void) {
    if (key_head != key_tail) {
        enum key_action action = key_queue[key_head];
        key_head = (uint8_t)((key_head + 1) % KEY_QUEUE_SIZE);
        return action;
    }

    static uint8_t extended = 0;
    uint8_t status = inb(0x64);
    if ((status & 0x01) == 0) {
        return KEY_NONE;
    }
    if (status & 0x20) {
        return KEY_NONE;
    }

    uint8_t sc = inb(0x60);
    if (sc == 0xE0) {
        extended = 1;
        return KEY_NONE;
    }
    if (sc & 0x80) {
        return KEY_NONE;
    }

    if (extended) {
        extended = 0;
        switch (sc) {
        case 0x48:
            return KEY_UP;
        case 0x50:
            return KEY_DOWN;
        case 0x4B:
            return KEY_LEFT;
        case 0x4D:
            return KEY_RIGHT;
        default:
            return KEY_NONE;
        }
    }

    switch (sc) {
    case 0x1C:
        return KEY_ENTER;
    case 0x01:
        return KEY_ESC;
    case 0x0F:
        return KEY_TAB;
    case 0x1F:
        return KEY_START;
    default:
        return KEY_NONE;
    }
}

void poll_mouse(struct ui_state *state, const struct framebuffer *fb) {
    static uint8_t packet[3];
    static uint8_t cycle = 0;
    uint8_t status = inb(0x64);
    if ((status & 0x01) == 0) {
        return;
    }
    if ((status & 0x20) == 0) {
        if (pending_mouse_valid) {
            int dx = pending_mouse_dx;
            int dy = pending_mouse_dy;
            state->mouse_buttons = pending_mouse_buttons;
            pending_mouse_dx = 0;
            pending_mouse_dy = 0;
            pending_mouse_valid = 0;
            state->mouse_x += dx;
            state->mouse_y -= dy;
        } else {
            return;
        }
    }

    packet[cycle++] = inb(0x60);
    if (cycle < 3) {
        return;
    }
    cycle = 0;

    int dx = (int8_t)packet[1];
    int dy = (int8_t)packet[2];
    state->mouse_buttons = packet[0] & 0x07;
    state->mouse_x += dx;
    state->mouse_y -= dy;
    if (state->mouse_x < 0) {
        state->mouse_x = 0;
    }
    if (state->mouse_y < 0) {
        state->mouse_y = 0;
    }
    if (state->mouse_x > (int)fb->width - 1) {
        state->mouse_x = (int)fb->width - 1;
    }
    if (state->mouse_y > (int)fb->height - 1) {
        state->mouse_y = (int)fb->height - 1;
    }

    if (pending_mouse_valid) {
        state->mouse_buttons = pending_mouse_buttons;
        state->mouse_x += pending_mouse_dx;
        state->mouse_y -= pending_mouse_dy;
        pending_mouse_dx = 0;
        pending_mouse_dy = 0;
        pending_mouse_valid = 0;
        if (state->mouse_x < 0) {
            state->mouse_x = 0;
        }
        if (state->mouse_y < 0) {
            state->mouse_y = 0;
        }
        if (state->mouse_x > (int)fb->width - 1) {
            state->mouse_x = (int)fb->width - 1;
        }
        if (state->mouse_y > (int)fb->height - 1) {
            state->mouse_y = (int)fb->height - 1;
        }
    }
}
