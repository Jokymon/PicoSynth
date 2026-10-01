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
                if (selectables[selected_widget_index]->active)
                {
                    selectables[selected_widget_index]->on_rotate(increment);
                }
                else
                {
                    select_increment(increment);
                }
                handle_rotation(event.rotation);
                break;
            case Gui::InputEvent::Type::Key:
                if ((event.key_id == KeyId::ROT_SWITCH) && event.pressed)
                {
                    auto event = selectables[selected_widget_index]->on_rot_push();
                    if (event.type == ReactionEvent::Type::Activate)
                    {
                        selectables[selected_widget_index]->active = true;
                    }
                    else if (event.type == ReactionEvent::Type::Deactivate)
                    {
                        selectables[selected_widget_index]->active = false;
                    }
                }
                else if (event.pressed)
                {
                    // TODO: add conversion function for this
                    size_t index = static_cast<size_t>(event.key_id) -
                            static_cast<size_t>(KeyId::KEY0);
                    if (push_buttons[index]!=nullptr)
                    {
                        push_buttons[index]->on_push();
                    }
                }
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
    }

    void Page::select_increment(int8_t increment)
    {
        selectables[selected_widget_index]->selected = false;
        selected_widget_index += (int16_t)selectables.size() + increment;
        selected_widget_index %= selectables.size();
        selectables[selected_widget_index]->selected = true;
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
