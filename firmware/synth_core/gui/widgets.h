#pragma once
#include "../hardware.h"
#include "types.h"
#include <stdint.h>
#include <string>

namespace Gui
{

class Widget {
public:
    UWORD x;
    UWORD y;
    UWORD width;
    UWORD height;

    // TODO: 'selected' would only apply to items than can be selected by a 
    // rotation with the dial. Other generic widgets may not necessarily be
    // selectable
    bool selected = false;
    bool pressed = false;

    virtual void draw() =0;
};

class NavItem : public Widget {
public:
    NavItem(UWORD x, UWORD y, const std::string& text);
    void draw() override;

    std::string text;
};

class RotaryWidget : public Widget {

};

class ButtonWidget : public Widget {

};

}