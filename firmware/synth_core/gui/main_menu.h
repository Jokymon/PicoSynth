#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "gui.h"
#include <stddef.h>
#include <stdint.h>

class MainMenu : public Gui::Page
{
public:
    MainMenu();
    void draw() override;

protected:
    void handle_rotation(RotaryDir direction) override;
    void handle_key_press(KeyId key, bool pressed) override;

private:
    uint8_t menu_index = 0;
};

#endif