#pragma once
#include "../hardware.h"
#include <stdint.h>

namespace Gui
{

struct InputEvent
{
    enum class Type : uint8_t
    {
        Rotation,
        Key,
    };

    static InputEvent from_rotation(RotaryDir direction)
    {
        InputEvent event;
        event.type = Type::Rotation;
        event.rotation = direction;
        return event;
    }

    static InputEvent from_key(KeyId key_id, bool pressed)
    {
        InputEvent event;
        event.type = Type::Key;
        event.key_id = key_id;
        event.pressed = pressed;
        return event;
    }

    Type type;
    union
    {
        RotaryDir rotation;
        KeyId key_id;
    };
    bool pressed;
};

}
