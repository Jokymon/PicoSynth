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

    virtual void draw() =0;
};

struct Selectable {
    // TODO: We should make sure, that these states are only readable by derived
    // classes to make it clear that they are only reading clients of these
    // values. Only the `Page` class should actually set these values.
    bool selected = false;
    bool active = false;

    // Function that is called when the push-button of the rotary dial is
    // pressed
    virtual ReactionEvent on_rot_push() =0;
    virtual void on_rotate(int8_t increment) =0;
};

struct PushButton {
    size_t button_index = 0;
    bool pressed = false;

    // Function that is called when the assigned non-rotary button is pressed
    virtual void on_push() =0;
};

class NavItem : public Widget, public Selectable {
public:
    NavItem(UWORD x, UWORD y, const std::string& text);
    void draw() override;

    std::string text;

    ReactionEvent on_rot_push() override;
    void on_rotate(int8_t increment) override;
};

class TestButton : public Widget, public Selectable, public PushButton {
public:
    TestButton(size_t index, const std::string& text);
    void draw() override;

    std::string text;
    size_t my_number;

    ReactionEvent on_rot_push() override;
    void on_push() override;
    void on_rotate(int8_t increment) override;
};

}