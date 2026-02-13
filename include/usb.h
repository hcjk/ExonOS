#pragma once

#include <stdint.h>

#define USB_MAX_CONTROLLERS 8

struct usb_controller_info {
	uint8_t bus;
	uint8_t dev;
	uint8_t func;
	uint8_t class_code;
	uint8_t subclass;
	uint8_t prog_if;
	uint32_t bar0;
};

void usb_init(void);
void usb_poll(void);
uint32_t usb_controller_count(void);
const struct usb_controller_info *usb_controller_list(void);
