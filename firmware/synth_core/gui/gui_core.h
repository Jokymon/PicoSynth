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

/**
 * The ReactionEvent is returned by a (most likely) widget which is Selectable.
 * It encapsulates the intent of the widget for what should happen when pressing
 * the button of the rotary dial. The execution of this intent is then carried
 * out by the Page or the PageStack so that the widget doesn't need to have all
 * the required dependencies itself.
 * The event itself can be one of the following types:
 * - **Activate**; The widget wants to switch into a state which accepts rotary
 *   dial events. Most likely this is due to the widget switching into an
 *   editing mode and wants to use the events for modification.
 * - **Deactivate**; The widget wants to switch back from active state.
 * - **PageChange**; The widget requests the current page to change to a
 *   different page.
 */
struct ReactionEvent
{
    enum class Type : uint8_t
    {
        Activate,
        Deactivate,
        PageChange,
    };

    static ReactionEvent activate()
    {
        ReactionEvent event;
        event.type = Type::Activate;
        return event;
    }

    static ReactionEvent deactivate()
    {
        ReactionEvent event;
        event.type = Type::Deactivate;
        return event;
    }

    static ReactionEvent from_target_page(uint16_t target_page)
    {
        ReactionEvent event;
        event.type = Type::PageChange;
        event.target_page = target_page;
        return event;
    }

    Type type;
    union
    {
        uint16_t target_page;
    };
};

}
