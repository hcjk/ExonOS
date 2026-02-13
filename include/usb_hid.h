#pragma once

#include <stdint.h>

void usb_hid_init(void);
void usb_hid_poll(void);
void usb_hid_on_keyboard_report(const uint8_t *report, uint32_t len);
void usb_hid_on_mouse_report(const uint8_t *report, uint32_t len);
