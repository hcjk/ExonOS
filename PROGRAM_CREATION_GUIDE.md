# ExonOS Program Creation Guide (MagicUI + App Registry)

This guide explains how to create new programs (apps) for your OS using the current UI architecture.

The system is split into:
- **MagicUI core** (`src/MagicUI.c`, `include/magicui.h`) for shared UI drawing.
- **App registry** (`src/ui_apps.c`, `include/ui_apps.h`) for app list/menu/icons/open state/render dispatch.
- **App files** (`src/app_*.c`) where each program’s UI logic lives.
- **UI orchestrator** (`src/ui.c`) which handles mouse/keyboard/menu/drag globally.

---

## 1) Architecture Overview

## 1.1 MagicUI (UI primitives)
Use these core helpers instead of rewriting drawing logic in every app:
- `mui_draw_window(...)`
- `mui_draw_button(...)`
- `mui_draw_progress(...)`
- `mui_window_titlebar_rect(...)`
- `mui_window_close_rect(...)`
- `mui_theme_color(...)`

## 1.2 App Registry (single source of truth)
The registry controls:
- App IDs (`enum ui_app_id`)
- Start menu labels
- Desktop icons (optional)
- Open/close state mapping (`ui_state` fields)
- Window rect mapping
- App click handling dispatch
- App render dispatch (`ui_app_render`)

## 1.3 UI Main Loop
`src/ui.c` does not know app internals.
It only:
- Shows menu/icons
- Opens/closes apps
- Drags windows
- Calls `ui_app_handle_click(...)`
- Calls `ui_app_render(...)`

---

## 2) Quick Start (Add a New Program)

Example program name: **Notes**

### Step A — Add app file
Create `src/app_notes.c`.

### Step B — Declare function in header
In `include/ui_apps.h`, add:
- `void app_notes_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent);`
- Optional click handler declaration if you need interaction:
  - `int app_notes_handle_click(struct ui_state *state, int mouse_x, int mouse_y);`

### Step C — Add app ID
In `include/ui_apps.h`:
- Add `UI_APP_NOTES`
- Increment `UI_APP_COUNT`

### Step D — Add state fields
In `include/ui.h` (`struct ui_state`), add:
- `int notes_open;`
- `struct rect notes_rect;`

### Step E — Initialize defaults
In `src/ui.c` inside `ui_init(...)`, initialize:
- `state->notes_open = 0;`
- `state->notes_rect = (struct rect){ x, y, w, h };`

### Step F — Register in `src/ui_apps.c`
Update all required places:
1. `k_menu_labels`
2. `k_has_desktop_icon`
3. `k_desktop_icon_rects`
4. `k_desktop_icon_labels`
5. `ui_app_is_open(...)`
6. `ui_app_set_open(...)`
7. `ui_app_rect_mut(...)`
8. `ui_app_rect(...)`
9. `ui_app_render(...)`
10. (Optional) `ui_app_handle_click(...)`

### Step G — Add to build
In `Makefile`:
- Add `$(BUILD_DIR)/app_notes.o` to `OBJS`
- Add compile rule:
  - `$(BUILD_DIR)/app_notes.o: src/app_notes.c | $(BUILD_DIR)`
  - `\t$(CC) $(CFLAGS) -c $< -o $@`

### Step H — Build
Run:
- `make all CROSS=`
- `make build-iso CROSS=`

---

## 3) Copy-Paste App Template

Create `src/app_notes.c` with:

```c
#include <stdint.h>

#include "ui_apps.h"
#include "magicui.h"
#include "framebuffer.h"

void app_notes_render(const struct framebuffer *fb, const struct ui_state *state, uint32_t accent) {
    mui_draw_window(fb, state->notes_rect, "Notes", accent);

    int x = state->notes_rect.x + 16;
    int y = state->notes_rect.y + 40;

    fb_draw_string(fb, x, y, "Notes App", rgb(60, 60, 60), rgb(230, 234, 240));
    y += 16;
    fb_draw_string(fb, x, y, "- Add your UI here", rgb(60, 60, 60), rgb(230, 234, 240));
    y += 16;

    struct rect btn = { x, y + 8, 120, 24 };
    mui_draw_button(fb, btn, "Example", accent, rgb(255, 255, 255));

    struct rect progress = { x, y + 44, state->notes_rect.w - 32, 10 };
    mui_draw_progress(fb, progress, 3, 10, accent, rgb(180, 185, 195));
}
```

Optional click handler pattern:

```c
int app_notes_handle_click(struct ui_state *state, int mouse_x, int mouse_y) {
    struct rect btn = { state->notes_rect.x + 16, state->notes_rect.y + 64, 120, 24 };
    if (point_in_rect(mouse_x, mouse_y, btn)) {
        // Update app state here
        return 1;
    }
    return 0;
}
```

---

## 4) Integration Checklist (Do Not Skip)

Before building, verify all are done:
- [ ] New enum ID in `include/ui_apps.h`
- [ ] `UI_APP_COUNT` updated
- [ ] New render function declaration in `include/ui_apps.h`
- [ ] New `open` and `rect` fields in `include/ui.h`
- [ ] `ui_init(...)` defaults in `src/ui.c`
- [ ] App added to all registry arrays in `src/ui_apps.c`
- [ ] App mapped in `ui_app_is_open`, `ui_app_set_open`, `ui_app_rect`, `ui_app_rect_mut`
- [ ] App mapped in `ui_app_render`
- [ ] Makefile object + rule added

---

## 5) Common Mistakes and Fixes

## 5.1 Linker error: multiple definition
Example:
- `multiple definition of 'app_apps_render'`

Cause:
- Two `.c` files define the same global function name.

Fix:
- Give every app unique function names (e.g., `app_notes_render`, not `app_apps_render`).

## 5.2 App appears in menu but does not open
Cause:
- Missing `ui_app_set_open(...)` mapping in `src/ui_apps.c`.

Fix:
- Add switch case for your app and set `state->your_open = value;`.

## 5.3 App opens but window position is broken
Cause:
- Missing `ui_app_rect(...)`/`ui_app_rect_mut(...)` mapping.

Fix:
- Return `state->your_rect` in both functions.

## 5.4 Build fails: undefined reference to app render
Cause:
- App declared but not implemented, or Makefile not compiling new file.

Fix:
- Implement the function in your app `.c` and add object rule in `Makefile`.

---

## 6) UI/Code Style Rules (Recommended)

- Keep app-specific logic inside `src/app_<name>.c`.
- Keep generic UI drawing in MagicUI only.
- Keep `src/ui.c` generic (no hardcoded per-app internals).
- Keep all app registration in `src/ui_apps.c`.
- Use unique symbol names per app.

---

## 7) Current Reference Files

- Core UI primitives:
  - `include/magicui.h`
  - `src/MagicUI.c`
- Registry:
  - `include/ui_apps.h`
  - `src/ui_apps.c`
- Global UI state:
  - `include/ui.h`
- UI loop:
  - `src/ui.c`
- Existing app examples:
  - `src/app_apps.c`
  - `src/app_settings.c`
  - `src/app_files.c`
  - `src/app_usb.c`
  - `src/app_test.c`

---

## 8) Build Commands

- Build kernel binary:
  - `make all CROSS=`
- Build bootable ISO:
  - `make build-iso CROSS=`
- Clean:
  - `make clean`

(Use your cross toolchain prefix if needed, for example default `i686-elf-`.)

---

## 9) Optional Next Upgrade (Future)

If you want easier app coding later, you can add a tiny scripting VM (for example Lua or a custom bytecode) that calls MagicUI wrappers.
For now, this C app-registry model is the most stable and easiest to debug in a freestanding kernel.
