#include "gui.h"
extern "C"
{
#include "lcd_1in44.h"
#include "GUI_Paint.h"
}
#include "hardware.h"
#include <array>
#include <stdio.h>

namespace Gui
{
    namespace
    {
        const uint16_t BUTTON_BACKGROUND_COLOR_IDLE = BLUE;
        const uint16_t BUTTON_BACKGROUND_COLOR_PRESSED = CYAN;
        const uint16_t BUTTON_BORDER_COLOR_SELECTED = 0x781F;
    }

    RotaryDir map_rotation(hw_rotary_dir_t direction)
    {
        return direction == HW_ROTARY_CW ? RotaryDir::CW : RotaryDir::CCW;
    }

    KeyId map_key(hw_key_id_t key)
    {
        switch (key) {
            case HW_KEY_ROTARY_SWITCH:
                return KeyId::ROT_SWITCH;
            case HW_KEY_BUTTON_1:
                return KeyId::KEY0;
            case HW_KEY_BUTTON_2:
                return KeyId::KEY1;
            case HW_KEY_BUTTON_3:
                return KeyId::KEY2;
            case HW_KEY_BUTTON_4:
                return KeyId::KEY3;
            default:
                return KeyId::KEY0;
        }
    }

    void Page::handle_event(hw_event_t& event)
    {
        switch (event.type) {
            case HW_EVENT_ROTATION:
                handle_rotation(map_rotation(event.data.rotation.direction));
                break;
            case HW_EVENT_KEY:
                bool pressed = false;
                if (event.data.key.state == HW_KEY_PRESSED) {
                    pressed = true;
                }

                KeyId key = map_key(event.data.key.key);
                handle_key_press(key, pressed);
                break;
        }
    }

    Button::Button(ButtonId id, const char* label)
        : id(id), label(label), pressed(false), active(false), selected(false)
    {
    }

    void Button::draw()
    {
        static const UWORD BUTTON_WIDTH = LCD_1IN44.HEIGHT / 4;

        uint16_t background_color = pressed ? BUTTON_BACKGROUND_COLOR_PRESSED : BUTTON_BACKGROUND_COLOR_IDLE;

        Paint_DrawRectangle(id * BUTTON_WIDTH, LCD_1IN44.HEIGHT - BUTTON_WIDTH,
                            (id + 1) * BUTTON_WIDTH, LCD_1IN44.HEIGHT,
                            background_color, DOT_PIXEL_1X1,
                            DRAW_FILL_FULL);

        if (selected)
        {
            Paint_DrawRectangle(id * BUTTON_WIDTH+1, LCD_1IN44.HEIGHT - BUTTON_WIDTH+1,
                                ((id + 1) * BUTTON_WIDTH)-2, LCD_1IN44.HEIGHT-2,
                                BUTTON_BORDER_COLOR_SELECTED, DOT_PIXEL_2X2,
                                DRAW_FILL_EMPTY);
        }

        if (label)
        {
            uint16_t text_background_color = background_color;
            if (active) {
                text_background_color = BLACK;
            }

            Paint_DrawString_EN(id * BUTTON_WIDTH + 2, LCD_1IN44.HEIGHT - BUTTON_WIDTH + 8, label, &Font12, WHITE, text_background_color);
        }
    }
}
