#include "main_menu.h"
#include "gui/colors.h"
#include <array>

extern "C" {
#include "GUI_Paint.h"
#include "lcd_1in44.h"
}

const int32_t TITLE_HEIGHT = 14;

const int32_t TEXT_HEIGHT = 12;
const int32_t TEXT_SPACING = 6;
const int32_t MENU_MARGIN_LEFT = 6;
const int32_t MENU_MARGIN_TOP = 2;

MainMenu::MainMenu()
{
    append_widget(std::make_unique<Gui::NavItem>(
        2, TITLE_HEIGHT+MENU_MARGIN_TOP,
        "Voices"
    ));
    append_widget(std::make_unique<Gui::NavItem>(
        2, TITLE_HEIGHT+MENU_MARGIN_TOP+TEXT_HEIGHT+TEXT_SPACING,
        "Sequencer"
    ));
    append_widget(std::make_unique<Gui::NavItem>(
        2, TITLE_HEIGHT+MENU_MARGIN_TOP+2*(TEXT_HEIGHT+TEXT_SPACING),
        "Settings"
    ));
}

void MainMenu::draw()
{
    Paint_DrawString_EN(MENU_MARGIN_LEFT, 2, "MAIN MENU", &Font12, TEXT_PRIMARY, BG_BLACK);

    Paint_DrawLine(0, TITLE_HEIGHT, LCD_1IN44.WIDTH-1, TITLE_HEIGHT, BG_SURFACE, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Gui::Page::draw();
}

void MainMenu::handle_rotation(RotaryDir direction)
{
}

void MainMenu::handle_key_press(KeyId key, bool pressed)
{
}
