#include "gui.h"
#include "gui/colors.h"
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
        const uint16_t BUTTON_BACKGROUND_COLOR_IDLE = BG_SURFACE;
        const uint16_t BUTTON_BACKGROUND_COLOR_SELECTED = BG_SURFACE;
        const uint16_t BUTTON_BACKGROUND_COLOR_PRESSED = PRIMARY_ACCENT;

        const uint16_t BUTTON_BORDER_COLOR_SELECTED = PRIMARY_ACCENT;
        const uint16_t BUTTON_BORDER_COLOR_PRESSED = PRIMARY_ACCENT;
    
        const uint16_t BUTTON_TEXT_COLOR = TEXT_PRIMARY;
        const uint16_t BUTTON_TEXT_COLOR_SELECTED = PRIMARY_ACCENT;
        const uint16_t BUTTON_TEXT_COLOR_PRESSED = BG_PANEL;
    }

    void Page::handle_event(Gui::InputEvent& event)
    {
        int8_t increment=0;
        switch (event.type) {
            case Gui::InputEvent::Type::Rotation:
                increment = event.rotation == RotaryDir::CCW ? -1 : +1;
                select_increment(increment);
                handle_rotation(event.rotation);
                break;
            case Gui::InputEvent::Type::Key:
                handle_key_press(event.key_id, event.pressed);
                break;
        }
    }

    void Page::draw()
    {
        for (const auto& widget : widgets)
        {
            widget->draw();
        }
    }

    void Page::append_widget(std::unique_ptr<Gui::Widget> widget)
    {
        widgets.push_back(std::move(widget));
        if (widgets.size()==1)
        {
            // Make sure, that the first item is immediately selected
            widgets[0]->selected = true;
        }
    }

    void Page::select_increment(int8_t increment)
    {
        widgets[selected_widget_index]->selected = false;
        selected_widget_index += (int16_t)widgets.size() + increment;
        selected_widget_index %= widgets.size();
        widgets[selected_widget_index]->selected = true;
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
