#include "usb_hid.h"
#include "input.h"

static uint8_t last_keys[6];

static int key_in_last(uint8_t code) {
	for (int i = 0; i < 6; ++i) {
		if (last_keys[i] == code) {
			return 1;
		}
	}
	return 0;
}

static enum key_action hid_key_to_action(uint8_t code) {
	switch (code) {
	case 0x52:
		return KEY_UP;
	case 0x51:
		return KEY_DOWN;
	case 0x50:
		return KEY_LEFT;
	case 0x4F:
		return KEY_RIGHT;
	case 0x28:
		return KEY_ENTER;
	case 0x29:
		return KEY_ESC;
	case 0x2B:
		return KEY_TAB;
	case 0x16:
		return KEY_START;
	default:
		return KEY_NONE;
	}
}

void usb_hid_init(void) {
	for (int i = 0; i < 6; ++i) {
		last_keys[i] = 0;
	}
}

void usb_hid_on_keyboard_report(const uint8_t *report, uint32_t len) {
	if (!report || len < 8) {
		return;
	}
	const uint8_t *keys = report + 2;
	for (int i = 0; i < 6; ++i) {
		uint8_t code = keys[i];
		if (code == 0) {
			continue;
		}
		if (!key_in_last(code)) {
			enum key_action action = hid_key_to_action(code);
			if (action != KEY_NONE) {
				input_inject_key(action);
			}
		}
	}
	for (int i = 0; i < 6; ++i) {
		last_keys[i] = keys[i];
	}
}

void usb_hid_on_mouse_report(const uint8_t *report, uint32_t len) {
	if (!report || len < 3) {
		return;
	}
	int8_t dx = (int8_t)report[1];
	int8_t dy = (int8_t)report[2];
	uint8_t buttons = report[0] & 0x07;
	input_inject_mouse(dx, dy, buttons);
}

void usb_hid_poll(void) {
}
