#include "usb.h"
#include "ehci.h"
#include "log.h"
#include "pci.h"
#include "usb_hid.h"

static struct usb_controller_info controllers[USB_MAX_CONTROLLERS];
static uint32_t controller_count;
static struct ehci_controller ehci_ctrls[USB_MAX_CONTROLLERS];
static uint32_t ehci_count;

struct usb_device {
    uint8_t addr;
    uint8_t interface_num;
    uint8_t protocol;
    uint8_t report_len;
    uint8_t is_keyboard;
    uint8_t is_mouse;
};

static struct usb_device hid_device;

static int usb_get_device_descriptor(struct ehci_controller *ctrl, uint8_t addr, uint8_t *buf, uint32_t len) {
    uint8_t setup[8] = { 0x80, 0x06, 0x00, 0x01, 0x00, 0x00, (uint8_t)len, 0x00 };
    return ehci_control_transfer(ctrl, addr, 0, 64, setup, buf, len, 1);
}

static int usb_set_address(struct ehci_controller *ctrl, uint8_t addr) {
    uint8_t setup[8] = { 0x00, 0x05, addr, 0x00, 0x00, 0x00, 0x00, 0x00 };
    return ehci_control_transfer(ctrl, 0, 0, 64, setup, 0, 0, 0);
}

static int usb_get_config_descriptor(struct ehci_controller *ctrl, uint8_t addr, uint8_t *buf, uint32_t len) {
    uint8_t setup[8] = { 0x80, 0x06, 0x00, 0x02, 0x00, 0x00, (uint8_t)len, 0x00 };
    return ehci_control_transfer(ctrl, addr, 0, 64, setup, buf, len, 1);
}

static int usb_set_configuration(struct ehci_controller *ctrl, uint8_t addr, uint8_t cfg) {
    uint8_t setup[8] = { 0x00, 0x09, cfg, 0x00, 0x00, 0x00, 0x00, 0x00 };
    return ehci_control_transfer(ctrl, addr, 0, 64, setup, 0, 0, 0);
}

static int usb_set_protocol(struct ehci_controller *ctrl, uint8_t addr, uint8_t iface, uint8_t protocol) {
    uint8_t setup[8] = { 0x21, 0x0B, protocol, 0x00, iface, 0x00, 0x00, 0x00 };
    return ehci_control_transfer(ctrl, addr, 0, 64, setup, 0, 0, 0);
}

static int usb_get_report(struct ehci_controller *ctrl, uint8_t addr, uint8_t iface, uint8_t len, uint8_t *buf) {
    uint8_t setup[8] = { 0xA1, 0x01, 0x00, 0x01, iface, 0x00, len, 0x00 };
    return ehci_control_transfer(ctrl, addr, 0, 64, setup, buf, len, 1);
}

static void usb_device_cb(uint8_t bus, uint8_t dev, uint8_t func,
                          uint8_t class_code, uint8_t subclass,
                          uint8_t prog_if, void *ctx) {
    (void)ctx;
    if (class_code == 0x0C && subclass == 0x03) {
        if (controller_count < USB_MAX_CONTROLLERS) {
            struct usb_controller_info *info = &controllers[controller_count++];
            info->bus = bus;
            info->dev = dev;
            info->func = func;
            info->class_code = class_code;
            info->subclass = subclass;
            info->prog_if = prog_if;
            info->bar0 = pci_read_config32(bus, dev, func, 0x10);
        }
        if (prog_if == 0x20 && ehci_count < USB_MAX_CONTROLLERS) {
            uint32_t bar0 = pci_read_config32(bus, dev, func, 0x10);
            if ((bar0 & 0x1u) == 0 && bar0 != 0xFFFFFFFFu && bar0 != 0) {
                uint32_t bar_type = bar0 & 0x6u;
                if (bar_type == 0x4u) {
                    uint32_t bar1 = pci_read_config32(bus, dev, func, 0x14);
                    if (bar1 != 0) {
                        return;
                    }
                }
                if (ehci_init(&ehci_ctrls[ehci_count], bar0)) {
                    ehci_count++;
                }
            }
        }
        log_puts("USB ctrl: bus=");
        log_dec32(bus);
        log_puts(" dev=");
        log_dec32(dev);
        log_puts(" func=");
        log_dec32(func);
        log_puts(" prog_if=");
        log_hex32(prog_if);
        log_puts("\n");
    }
}

void usb_init(void) {
    controller_count = 0;
    ehci_count = 0;
    log_puts("USB scan...\n");
    pci_scan_bus0(usb_device_cb, 0);
    usb_hid_init();

    hid_device.addr = 0;
    hid_device.interface_num = 0;
    hid_device.protocol = 0;
    hid_device.report_len = 0;
    hid_device.is_keyboard = 0;
    hid_device.is_mouse = 0;

    if (ehci_count == 0) {
        return;
    }

    struct ehci_controller *ctrl = &ehci_ctrls[0];
    uint8_t dev_desc[18];
    if (!usb_get_device_descriptor(ctrl, 0, dev_desc, 8)) {
        log_puts("USB: no device desc\n");
        return;
    }

    if (!usb_set_address(ctrl, 1)) {
        log_puts("USB: set address failed\n");
        return;
    }

    hid_device.addr = 1;
    if (!usb_get_device_descriptor(ctrl, hid_device.addr, dev_desc, 18)) {
        log_puts("USB: full desc failed\n");
        return;
    }

    uint8_t cfg_desc[64];
    if (!usb_get_config_descriptor(ctrl, hid_device.addr, cfg_desc, 9)) {
        log_puts("USB: cfg header failed\n");
        return;
    }
    uint16_t total_len = (uint16_t)(cfg_desc[2] | (cfg_desc[3] << 8));
    if (total_len > sizeof(cfg_desc)) {
        total_len = sizeof(cfg_desc);
    }
    if (!usb_get_config_descriptor(ctrl, hid_device.addr, cfg_desc, (uint8_t)total_len)) {
        log_puts("USB: cfg read failed\n");
        return;
    }

    uint8_t cfg_value = cfg_desc[5];
    if (!usb_set_configuration(ctrl, hid_device.addr, cfg_value)) {
        log_puts("USB: set config failed\n");
        return;
    }

    uint32_t idx = 9;
    while (idx + 2 < total_len) {
        uint8_t len = cfg_desc[idx];
        uint8_t type = cfg_desc[idx + 1];
        if (len == 0) {
            break;
        }
        if (type == 0x04 && len >= 9) {
            uint8_t iface = cfg_desc[idx + 2];
            uint8_t iface_class = cfg_desc[idx + 5];
            uint8_t iface_sub = cfg_desc[idx + 6];
            uint8_t iface_proto = cfg_desc[idx + 7];
            if (iface_class == 0x03 && iface_sub == 0x01) {
                hid_device.interface_num = iface;
                hid_device.protocol = iface_proto;
                hid_device.is_keyboard = (iface_proto == 1);
                hid_device.is_mouse = (iface_proto == 2);
            }
        } else if (type == 0x21 && len >= 9) {
            uint16_t rep_len = (uint16_t)(cfg_desc[idx + 7] | (cfg_desc[idx + 8] << 8));
            if (rep_len > 0 && rep_len <= 64) {
                hid_device.report_len = (uint8_t)rep_len;
            }
        }
        idx += len;
    }

    if (hid_device.report_len == 0) {
        hid_device.report_len = hid_device.is_keyboard ? 8 : 3;
    }

    if (hid_device.is_keyboard || hid_device.is_mouse) {
        usb_set_protocol(ctrl, hid_device.addr, hid_device.interface_num, 0);
        log_puts("USB HID ready\n");
    }
}

void usb_poll(void) {
    if (ehci_count == 0) {
        return;
    }
    if (hid_device.addr == 0) {
        return;
    }
    uint8_t report[64];
    struct ehci_controller *ctrl = &ehci_ctrls[0];
    if (usb_get_report(ctrl, hid_device.addr, hid_device.interface_num, hid_device.report_len, report)) {
        if (hid_device.is_keyboard) {
            usb_hid_on_keyboard_report(report, hid_device.report_len);
        } else if (hid_device.is_mouse) {
            usb_hid_on_mouse_report(report, hid_device.report_len);
        }
    }
}

uint32_t usb_controller_count(void) {
    return controller_count;
}

const struct usb_controller_info *usb_controller_list(void) {
    return controllers;
}
