#ifndef GUI_H
#define GUI_H

#include "hardware.h"
#include "gui/gui_core.h"
#include "gui/widgets.h"
#include <concepts>
#include <memory>
#include <vector>

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
            virtual void draw();

        protected:
            virtual void handle_rotation(RotaryDir direction) =0;
            virtual void handle_key_press(KeyId key, bool pressed) =0;

            void append_widget(std::unique_ptr<Gui::Widget> widget);

            template <typename T>
                requires
                    std::derived_from<T, Gui::Widget> && 
                    std::derived_from<T, Gui::Selectable>
            void append_selectable_widget(std::unique_ptr<T> widget)
            {
                selectables.push_back(widget.get());
                if (selectables.size()==1)
                {
                    // Make sure, that the first item is immediately selected
                    selectables[0]->selected = true;
                }

                append_widget(std::move(widget));
            }

        private:
            void select_increment(int8_t increment);

        private:
            std::vector<std::unique_ptr<Gui::Widget>> widgets;
            std::vector<Gui::Selectable*> selectables;
            size_t selected_widget_index =0;
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
