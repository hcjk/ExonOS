#include <stdint.h>

#include "ui_apps.h"
#include "magicui.h"
#include "framebuffer.h"

void app_test_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent) {
    mui_draw_window(fb, state->test_rect, "Test App", accent);
    fb_draw_string(fb, state->test_rect.x + 20, state->test_rect.y + 40, "Hello from Test App!", rgb(60, 60, 60), rgb(230, 234, 240));
    fb_draw_string(fb, state->test_rect.x + 20, state->test_rect.y + 60, "This is your custom app file.", rgb(60, 60, 60), rgb(230, 234, 240));
}
