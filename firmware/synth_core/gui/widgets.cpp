#include "gui/widgets.h"
#include "gui/colors.h"

extern "C" {
    #include "GUI_Paint.h"
    #include "lcd_1in44.h"
}

namespace Gui {

    namespace
    {
        const int32_t NAVITEM_MARGIN = 2;
        const int32_t NAVITEM_PADDING_TOP = 3;
        const int32_t NAVITEM_PADDING_LEFT = 4;

        const int32_t TEXT_HEIGHT = 12;
        const int32_t TEXT_SPACING = 6;
        const int32_t MENU_MARGIN_LEFT = 6;
        const int32_t MENU_MARGIN_TOP = 6;
    }

NavItem::NavItem(UWORD x, UWORD y, const std::string& text)
: text(text)
{
    this->x = x;
    this->y = y;
    width = LCD_1IN44_WIDTH-x-NAVITEM_MARGIN;
    height = TEXT_HEIGHT+TEXT_SPACING;
}

void NavItem::draw()
{
    UWORD text_color = TEXT_PRIMARY;
    if (selected)
    {
        text_color = PRIMARY_ACCENT;
        Paint_DrawRectangle(x, y,
                            x+width, y+height,
                            PRIMARY_ACCENT, DOT_PIXEL_1X1,
                            DRAW_FILL_EMPTY);
    }

    Paint_DrawString_EN(
        x+NAVITEM_PADDING_LEFT, y+NAVITEM_PADDING_TOP,
        text.c_str(),
        &Font12,
        text_color,
        BG_PANEL);
}

ReactionEvent NavItem::on_rot_push()
{
    return ReactionEvent::from_target_page(1);
}

void NavItem::on_rotate(int8_t increment)
{
    // NavItem is not expected to ever become active and thus
    // should also never get any rotation events.

    // TODO: Should we separate selectables which can be activated
    // and those which only ever navigate? How would we model this
    // differently?
}


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

TestButton::TestButton(size_t index, const std::string& text)
: text(text)
{
    static const UWORD BUTTON_WIDTH = LCD_1IN44.WIDTH / 4;

    this->button_index = index;

    this->x = index * BUTTON_WIDTH;
    this->y = LCD_1IN44.HEIGHT - BUTTON_WIDTH;
    this->width = BUTTON_WIDTH;
    this->height = BUTTON_WIDTH;

    this->text = std::to_string(index);
}

void TestButton::draw()
{
    uint16_t background_color = pressed ? BUTTON_BACKGROUND_COLOR_PRESSED : BUTTON_BACKGROUND_COLOR_IDLE;
    if (active) {
        background_color = BUTTON_BACKGROUND_COLOR_PRESSED;
    }

    Paint_DrawRectangle(x, y,
                        x+width, y+height,
                        background_color, DOT_PIXEL_1X1,
                        DRAW_FILL_FULL);

    if (selected)
    {
        uint16_t border_color = BUTTON_BORDER_COLOR_SELECTED;
        if (active) {
            border_color = BUTTON_BORDER_COLOR_PRESSED;
        }

        Paint_DrawRectangle(x, y,
                            x+width, y+height,
                            border_color, DOT_PIXEL_2X2,
                            DRAW_FILL_EMPTY);
    }

    uint16_t text_background_color = background_color;

    uint16_t text_color = BUTTON_TEXT_COLOR;
    if (selected) {
        text_color = BUTTON_TEXT_COLOR_SELECTED;
    }
    if (active || pressed) {
        text_color = BUTTON_TEXT_COLOR_PRESSED;
    }

    Paint_DrawString_EN(x + 4, y + 9, text.c_str(),
                        &Font12, text_color, text_background_color);
}

ReactionEvent TestButton::on_rot_push()
{
    if (active)
    {
        return ReactionEvent::deactivate();
    }
    else
    {
        return ReactionEvent::activate();
    }
}

void TestButton::on_push()
{
    printf("You just pushed that button\n");
}

void TestButton::on_rotate(int8_t increment)
{
    my_number = (my_number + increment + 5) % 5;
    text = std::to_string(my_number);
}

}