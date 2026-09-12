#ifndef GUI_H
#define GUI_H

#include "hardware.h"

namespace Gui
{

    enum ButtonId
    {
        Key0 = 0,
        Key1 = 1,
        Key2 = 2,
        Key3 = 3
    };

    RotaryDir map_rotation(hw_rotary_dir_t direction);
    KeyId map_key(hw_key_id_t key);

    class Page {
        public:
            void handle_event(hw_event_t& event);
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
