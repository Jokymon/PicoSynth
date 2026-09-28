#pragma once
#include "../hardware.h"
#include "gui/gui_core.h"
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
    // TODO: We should make sure, that these states are only readable by derived
    // classes to make it clear that they are only reading clients of these
    // values. Only the `Page` class should actually set these values.
    bool selected = false;
    bool active = false;

    virtual ReactionEvent on_push() =0;
    virtual void on_rotate(int8_t increment) =0;
};

class NavItem : public Widget, public Selectable {
public:
    NavItem(UWORD x, UWORD y, const std::string& text);
    void draw() override;

    std::string text;

    ReactionEvent on_push() override;
    void on_rotate(int8_t increment) override;
};

class TestButton : public Widget, public Selectable {
public:
    TestButton(size_t index, const std::string& text);
    void draw() override;

    std::string text;
    size_t index = 0;

    ReactionEvent on_push() override;
    void on_rotate(int8_t increment) override;
};

}