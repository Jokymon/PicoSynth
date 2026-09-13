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
        #define BG_BLACK 0x0862
        #define BG_PANEL 0x18c4
        #define BG_SURFACE 0x2147

        #define TEXT_PRIMARY 0xe73d
        #define TEXT_SECONDARY 0x9d56
        #define TEXT_DISABLED 0x5b0e

        #define PRIMARY_ACCENT 0x073f
        #define SECONDARY_ACCENT 0x7a7f
        #define TERTIARY_ACCENT 0xf96d
        #define WARNING 0xfe8c

        #define STATUS_SUCCESS 0x4ef0
        #define STATUS_ERROR 0xf38e
        #define STATUS_INFO 0x3dfe
    
        const uint16_t BUTTON_BACKGROUND_COLOR_IDLE = BG_SURFACE;
        const uint16_t BUTTON_BACKGROUND_COLOR_SELECTED = BG_SURFACE;
        const uint16_t BUTTON_BACKGROUND_COLOR_PRESSED = PRIMARY_ACCENT;

        const uint16_t BUTTON_BORDER_COLOR_SELECTED = PRIMARY_ACCENT;
        const uint16_t BUTTON_BORDER_COLOR_PRESSED = PRIMARY_ACCENT;
    
        const uint16_t BUTTON_TEXT_COLOR = TEXT_PRIMARY;
        const uint16_t BUTTON_TEXT_COLOR_SELECTED = PRIMARY_ACCENT;
        const uint16_t BUTTON_TEXT_COLOR_PRESSED = BG_PANEL;
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
        if (active) {
            background_color = BUTTON_BACKGROUND_COLOR_PRESSED;
        }

        Paint_DrawRectangle(id * BUTTON_WIDTH, LCD_1IN44.HEIGHT - BUTTON_WIDTH,
                            (id + 1) * BUTTON_WIDTH, LCD_1IN44.HEIGHT,
                            background_color, DOT_PIXEL_1X1,
                            DRAW_FILL_FULL);

        if (selected)
        {
            uint16_t border_color = BUTTON_BORDER_COLOR_SELECTED;
            if (active) {
                border_color = BUTTON_BORDER_COLOR_PRESSED;
            }

            Paint_DrawRectangle(id * BUTTON_WIDTH+1, LCD_1IN44.HEIGHT - BUTTON_WIDTH+1,
                                ((id + 1) * BUTTON_WIDTH)-2, LCD_1IN44.HEIGHT-2,
                                border_color, DOT_PIXEL_2X2,
                                DRAW_FILL_EMPTY);
        }

        if (label)
        {
            uint16_t text_background_color = background_color;

            uint16_t text_color = BUTTON_TEXT_COLOR;
            if (selected) {
                text_color = BUTTON_TEXT_COLOR_SELECTED;
            }
            if (active || pressed) {
                text_color = BUTTON_TEXT_COLOR_PRESSED;
            }

            Paint_DrawString_EN(id * BUTTON_WIDTH + 4, LCD_1IN44.HEIGHT - BUTTON_WIDTH + 9, label, &Font12, text_color, text_background_color);
        }
    }
}
