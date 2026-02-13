#include <stdint.h>

#include "ui_apps.h"
#include "magicui.h"
#include "framebuffer.h"

void app_apps_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent) {
    mui_draw_window(fb, state->apps_rect, "Apps", accent);
    fb_draw_string(fb, state->apps_rect.x + 20, state->apps_rect.y + 40, "Calculator", rgb(60, 60, 60), rgb(230, 234, 240));
    fb_draw_string(fb, state->apps_rect.x + 20, state->apps_rect.y + 60, "Notepad", rgb(60, 60, 60), rgb(230, 234, 240));
}
