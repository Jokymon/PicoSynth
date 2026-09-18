#ifndef GUI_H
#define GUI_H

#include "hardware.h"
#include "gui/gui_core.h"

namespace Gui
{

    enum ButtonId
    {
        Key0 = 0,
        Key1 = 1,
        Key2 = 2,
        Key3 = 3
    };

    class Page {
        public:
            void handle_event(Gui::InputEvent& event);
            virtual void draw() =0;

        protected:
            virtual void handle_rotation(RotaryDir direction) =0;
            virtual void handle_key_press(KeyId key, bool pressed) =0;
    };

    struct Button {
        Button(ButtonId id, const char* label=nullptr);

        ButtonId id;
        const char* label;
        bool pressed;
        bool active;
        bool selected;

        void draw();
    };
}

#endif
