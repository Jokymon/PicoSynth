#include "synth_c_api.h"

#include "GUI_Paint.h"
#include "hardware.h"

static void draw_status(UWORD *screen, const char *status) {
    Paint_Clear(BLACK);
    Paint_DrawString_EN(0, 40, "Synth", &Font12, WHITE, BLACK);
    Paint_DrawString_EN(0, 58, status, &Font12, WHITE, BLACK);
}

static const char *key_label(hw_key_id_t key) {
    switch (key) {
    case HW_KEY_BUTTON_1:
        return "Button 1";
    case HW_KEY_BUTTON_2:
        return "Button 2";
    case HW_KEY_BUTTON_3:
        return "Button 3";
    case HW_KEY_BUTTON_4:
        return "Button 4";
    case HW_KEY_ROTARY_SWITCH:
        return "Enter";
    default:
        return "Key ?";
    }
}

void synth_app_start(UWORD *screen) {
    if (screen == 0) {
        return;
    }

    init_hardware();

    Paint_NewImage((UBYTE *)screen, LCD_1IN44.WIDTH, LCD_1IN44.HEIGHT,
                   ROTATE_270, BLACK);

    Paint_SetScale(65);
    Paint_SetRotate(ROTATE_270);
    draw_status(screen, "Ready");
}

void synth_app_loop(UWORD *screen) {
    hw_event_t event;

    if (screen == 0 || !get_event(&event)) {
        return;
    }

    if (event.type == HW_EVENT_ROTATION) {
        draw_status(screen,
                    event.data.rotation.direction == HW_ROTARY_CW ? "Rot CW"
                                                                   : "Rot CCW");
    } else if (event.type == HW_EVENT_KEY &&
               event.data.key.state == HW_KEY_PRESSED) {
        draw_status(screen, key_label(event.data.key.key));
    }

    LCD_1IN44_Display(screen);
}
