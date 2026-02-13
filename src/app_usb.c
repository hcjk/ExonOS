#include <stdint.h>

#include "ui_apps.h"
#include "magicui.h"
#include "framebuffer.h"
#include "usb.h"

void app_usb_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent) {
    mui_draw_window(fb, state->usb_rect, "USB Manager", accent);
    int x = state->usb_rect.x + 16;
    int y = state->usb_rect.y + 40;
    fb_draw_string(fb, x, y, "Controllers:", rgb(60, 60, 60), rgb(230, 234, 240));
    y += 16;

    const struct usb_controller_info *list = usb_controller_list();
    uint32_t count = usb_controller_count();
    if (count == 0) {
        fb_draw_string(fb, x, y, "None found", rgb(60, 60, 60), rgb(230, 234, 240));
    }

    for (uint32_t i = 0; i < count; ++i) {
        char line[20];
        line[0] = 'B';
        line[1] = (char)('0' + (list[i].bus % 10));
        line[2] = ':';
        line[3] = 'D';
        line[4] = (char)('0' + (list[i].dev % 10));
        line[5] = ':';
        line[6] = 'F';
        line[7] = (char)('0' + (list[i].func % 10));
        line[8] = ' ';
        line[9] = 'P';
        line[10] = 'I';
        line[11] = '=';
        line[12] = '0';
        line[13] = 'x';
        line[14] = "0123456789ABCDEF"[(list[i].prog_if >> 4) & 0xF];
        line[15] = "0123456789ABCDEF"[list[i].prog_if & 0xF];
        line[16] = '\0';
        fb_draw_string(fb, x, y, line, rgb(60, 60, 60), rgb(230, 234, 240));
        y += 16;
    }
}
