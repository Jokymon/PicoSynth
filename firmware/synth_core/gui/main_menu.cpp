#include "main_menu.h"
#include "gui/colors.h"
#include <array>

extern "C" {
#include "GUI_Paint.h"
#include "lcd_1in44.h"
}

std::array<const char*, 3> MENU_ENTRIES = {
    "Voices",
    "Sequencer",
    "Settings"
};

const int32_t TITLE_HEIGHT = 14;

const int32_t TEXT_HEIGHT = 12;
const int32_t TEXT_SPACING = 6;
const int32_t MENU_MARGIN_LEFT = 6;
const int32_t MENU_MARGIN_TOP = 6;

MainMenu::MainMenu()
{
}

void MainMenu::draw()
{
    Paint_DrawString_EN(MENU_MARGIN_LEFT, 2, "MAIN MENU", &Font12, TEXT_PRIMARY, BG_BLACK);

    Paint_DrawLine(0, TITLE_HEIGHT, LCD_1IN44.WIDTH-1, TITLE_HEIGHT, BG_SURFACE, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    for (size_t i=0; i<MENU_ENTRIES.size(); i++)
    {
        UWORD text_color = TEXT_PRIMARY;
        if (i==menu_index)
        {
            text_color = PRIMARY_ACCENT;
            Paint_DrawRectangle(2, TITLE_HEIGHT+MENU_MARGIN_TOP+(TEXT_HEIGHT+TEXT_SPACING)*i - 2,
                                LCD_1IN44.WIDTH-2, TITLE_HEIGHT+(TEXT_HEIGHT+TEXT_SPACING)*(i+1)+2,
                                PRIMARY_ACCENT, DOT_PIXEL_1X1,
                                DRAW_FILL_EMPTY);
        }

        Paint_DrawString_EN(
            MENU_MARGIN_LEFT, TITLE_HEIGHT+MENU_MARGIN_TOP+(TEXT_HEIGHT+TEXT_SPACING)*i,
            MENU_ENTRIES[i],
            &Font12,
            text_color,
            BG_PANEL);
    }
}

void MainMenu::handle_rotation(RotaryDir direction)
{
    int8_t increment = direction == RotaryDir::CCW ? -1 : +1;
    menu_index = (menu_index + MENU_ENTRIES.size() + increment) % MENU_ENTRIES.size();
}

void MainMenu::handle_key_press(KeyId key, bool pressed)
{
}
