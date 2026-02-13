#include <stdint.h>

#include "ui_apps.h"
#include "magicui.h"
#include "framebuffer.h"

void app_files_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent) {
    mui_draw_window(fb, state->files_rect, "Folders", accent);
    fb_draw_string(fb, state->files_rect.x + 20, state->files_rect.y + 40, "Documents", rgb(60, 60, 60), rgb(230, 234, 240));
    fb_draw_string(fb, state->files_rect.x + 20, state->files_rect.y + 60, "Downloads", rgb(60, 60, 60), rgb(230, 234, 240));
}
