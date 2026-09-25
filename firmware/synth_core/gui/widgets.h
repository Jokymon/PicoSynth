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

    bool pressed = false;

    virtual void draw() =0;
};

struct Selectable {
    bool selected = false;
};

class NavItem : public Widget, public Selectable {
public:
    NavItem(UWORD x, UWORD y, const std::string& text);
    void draw() override;

    std::string text;
};

class TestButton : public Widget, public Selectable {
public:
    TestButton(size_t index, const std::string& text);
    void draw() override;

    std::string text;
};

}