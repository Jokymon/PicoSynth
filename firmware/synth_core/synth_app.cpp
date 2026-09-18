#include "synth_c_api.h"

extern "C" {
#include "GUI_Paint.h"
}

#include "gui.h"
#include "gui/gui_core.h"
#include "midi.h"
#include "synthesizer.h"

#include <array>
#include <cstdint>
#include <cstdio>

namespace {

class MainPage : public Gui::Page {
public:
    MainPage()
        : buttons{Gui::Button(Gui::ButtonId::Key0),
                  Gui::Button(Gui::ButtonId::Key1),
                  Gui::Button(Gui::ButtonId::Key2),
                  Gui::Button(Gui::ButtonId::Key3)} {
        select(0);
    }

    void select(uint8_t index) {
        buttons[selection].selected = false;
        buttons[index].selected = true;
        selection = index;
    }

    void draw() override {
        buttons[0].label = NOTES[key0_tone].name;
        buttons[1].label = NOTES[key1_tone].name;

        for (auto &button : buttons) {
            button.draw();
        }
    }

protected:
    void handle_rotation(RotaryDir direction) override {
        int8_t increment = direction == RotaryDir::CCW ? -1 : +1;
        switch (state) {
        case Selecting:
            select(static_cast<uint8_t>((selection + 4 + increment) % 4));
            break;
        case EditingKey0:
            key0_tone = clamp_midi_note(key0_tone + increment);
            break;
        case EditingKey1:
            key1_tone = clamp_midi_note(key1_tone + increment);
            break;
        default:
            break;
        }
    }

    void handle_key_press(KeyId key, bool pressed) override {
        switch (key) {
        case KeyId::ROT_SWITCH:
            if (!pressed) {
                return;
            }
            if (Selecting == state) {
                if (selection == 0) {
                    state = EditingKey0;
                    buttons[selection].active = true;
                } else if (selection == 1) {
                    state = EditingKey1;
                    buttons[selection].active = true;
                }
            } else {
                buttons[selection].active = false;
                state = Selecting;
            }
            break;
        case KeyId::KEY0:
            buttons[0].pressed = pressed;
            if (pressed) {
                Audio::Synthesizer::note_on(0, key0_tone, 64);
            } else {
                Audio::Synthesizer::note_off(0, key0_tone, 64);
            }
            break;
        case KeyId::KEY1:
            buttons[1].pressed = pressed;
            if (pressed) {
                Audio::Synthesizer::note_on(1, key1_tone, 64);
            } else {
                Audio::Synthesizer::note_off(1, key1_tone, 64);
            }
            break;
        case KeyId::KEY2:
            buttons[2].pressed = pressed;
            if (pressed) {
                Audio::Synthesizer::note_on(2, 72, 64);
            } else {
                Audio::Synthesizer::note_off(2, 72, 64);
            }
            break;
        case KeyId::KEY3:
            buttons[3].pressed = pressed;
            break;
        }
    }

private:
    static uint8_t clamp_midi_note(int value) {
        if (value < 0) {
            return 0;
        }
        if (value > 127) {
            return 127;
        }
        return static_cast<uint8_t>(value);
    }

    uint8_t key0_tone = 69;
    uint8_t key1_tone = 69;

    enum State { Selecting, EditingKey0, EditingKey1, EditingKey2 };
    State state = Selecting;
    uint8_t selection = 0;

    std::array<Gui::Button, 4> buttons;
};

MainPage *main_page = nullptr;

void draw_main_page(UWORD *screen) {
    Paint_Clear(BLACK);
    main_page->draw();
    LCD_1IN44_Display(screen);
}

} // namespace

extern "C" void synth_app_start(UWORD *screen) {
    if (screen == nullptr) {
        return;
    }

    init_hardware();
    Audio::Synthesizer::start();

    Paint_NewImage((UBYTE *)screen, LCD_1IN44.WIDTH, LCD_1IN44.HEIGHT,
                   ROTATE_270, BLACK);
    Paint_SetScale(65);
    Paint_SetRotate(ROTATE_270);
    Paint_Clear(BLACK);

    if (main_page == nullptr) {
        main_page = new MainPage();
    }

    draw_main_page(screen);
}

RotaryDir map_rotation(hw_rotary_dir_t direction)
{
    return direction == HW_ROTARY_CW ? RotaryDir::CW : RotaryDir::CCW;
}

KeyId map_key(hw_key_id_t key)
{
    switch (key) {
        case HW_KEY_ROTARY_SWITCH:
            return KeyId::ROT_SWITCH;
        case HW_KEY_BUTTON_1:
            return KeyId::KEY0;
        case HW_KEY_BUTTON_2:
            return KeyId::KEY1;
        case HW_KEY_BUTTON_3:
            return KeyId::KEY2;
        case HW_KEY_BUTTON_4:
            return KeyId::KEY3;
        default:
            return KeyId::KEY0;
    }
}

extern "C" void synth_app_loop(UWORD *screen) {
    if (screen == nullptr || main_page == nullptr) {
        return;
    }

    hw_event_t event;
    bool changed = false;
    while (get_event(&event)) {

        Gui::InputEvent input_event;
        switch (event.type) {
            case HW_EVENT_ROTATION:
                input_event = Gui::InputEvent::from_rotation(map_rotation(event.data.rotation.direction));
                break;
            case HW_EVENT_KEY:
                bool pressed = false;
                if (event.data.key.state == HW_KEY_PRESSED) {
                    pressed = true;
                }

                input_event = Gui::InputEvent::from_key(map_key(event.data.key.key), pressed);

                break;
        }
        main_page->handle_event(input_event);
        changed = true;
    }

    if (changed) {
        draw_main_page(screen);
    }
}
