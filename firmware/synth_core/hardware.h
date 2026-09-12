#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
enum class KeyId {
    ROT_SWITCH,
    KEY0,
    KEY1,
    KEY2,
    KEY3,
};

enum class RotaryDir {
    CW,
    CCW,
};

extern "C" {
#endif

typedef enum {
    HW_KEY_ROTARY_SWITCH = 0,
    HW_KEY_BUTTON_1 = 1,
    HW_KEY_BUTTON_2 = 2,
    HW_KEY_BUTTON_3 = 3,
    HW_KEY_BUTTON_4 = 4,
} hw_key_id_t;

typedef enum {
    HW_ROTARY_CW = 0,
    HW_ROTARY_CCW = 1,
} hw_rotary_dir_t;

typedef enum {
    HW_KEY_RELEASED = 0,
    HW_KEY_PRESSED = 1,
} hw_key_state_t;

typedef enum {
    HW_EVENT_ROTATION = 0,
    HW_EVENT_KEY = 1,
} hw_event_type_t;

typedef struct {
    hw_rotary_dir_t direction;
} hw_rotation_event_t;

typedef struct {
    hw_key_id_t key;
    hw_key_state_t state;
} hw_key_event_t;

typedef struct {
    hw_event_type_t type;
    union {
        hw_rotation_event_t rotation;
        hw_key_event_t key;
    } data;
} hw_event_t;

void init_hardware();
bool get_event(hw_event_t *event);

#ifdef __cplusplus
}
#endif

#endif
