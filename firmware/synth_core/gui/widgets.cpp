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

}