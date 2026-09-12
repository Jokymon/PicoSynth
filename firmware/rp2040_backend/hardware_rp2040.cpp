#include "hardware.h"

#include "hardware/gpio.h"
#include "pico/types.h"
#include "pico/util/queue.h"

#include <array>
#include <cstdint>

namespace {

const uint GPIO_KEY0 = 15;
const uint GPIO_KEY1 = 17;
const uint GPIO_KEY2 = 2;
const uint GPIO_KEY3 = 3;

const uint ROTARY_CLK = 0;
const uint ROTARY_DT = 1;
const uint ROTARY_SW = 5;

queue_t event_queue;
uint8_t encoder_state;

void init_event_queue() {
    queue_init(&event_queue, sizeof(hw_event_t), 10);
}

// Idea and some code taken from:
// https://github.com/miketeachman/micropython-rotary/blob/master/rotary.py
constexpr uint8_t DIR_CW = 0x10;
constexpr uint8_t DIR_CCW = 0x20;
constexpr uint8_t R_START = 0x0;
constexpr uint8_t R_CW_1 = 0x1;
constexpr uint8_t R_CW_2 = 0x2;
constexpr uint8_t R_CW_3 = 0x3;
constexpr uint8_t R_CCW_1 = 0x4;
constexpr uint8_t R_CCW_2 = 0x5;
constexpr uint8_t R_CCW_3 = 0x6;

constexpr uint8_t STATE_MASK = 0x7;
constexpr uint8_t DIR_MASK = 0x30;

void create_key_event(hw_key_id_t key_code, uint32_t events) {
    hw_event_t event = {};

    if (events & GPIO_IRQ_EDGE_FALL) {
        event.type = HW_EVENT_KEY;
        event.data.key.key = key_code;
        event.data.key.state = HW_KEY_PRESSED;
        queue_try_add(&event_queue, &event);
    } else if (events & GPIO_IRQ_EDGE_RISE) {
        event.type = HW_EVENT_KEY;
        event.data.key.key = key_code;
        event.data.key.state = HW_KEY_RELEASED;
        queue_try_add(&event_queue, &event);
    }
}

void irq_callback(uint gpio, uint32_t events) {
    switch (gpio) {
    case ROTARY_SW:
        create_key_event(HW_KEY_ROTARY_SWITCH, events);
        return;
    case GPIO_KEY0:
        create_key_event(HW_KEY_BUTTON_1, events);
        return;
    case GPIO_KEY1:
        create_key_event(HW_KEY_BUTTON_2, events);
        return;
    case GPIO_KEY2:
        create_key_event(HW_KEY_BUTTON_3, events);
        return;
    case GPIO_KEY3:
        create_key_event(HW_KEY_BUTTON_4, events);
        return;
    }

    static const std::array<std::array<uint8_t, 4>, 8> TRANSITION_TABLE = {{
        {R_CW_3, R_CW_2, R_CW_1, R_START},
        {static_cast<uint8_t>(R_CW_3 | DIR_CCW), R_START, R_CW_1, R_START},
        {static_cast<uint8_t>(R_CW_3 | DIR_CW), R_CW_2, R_START, R_START},
        {R_CW_3, R_CCW_2, R_CCW_1, R_START},
        {R_CW_3, R_CW_2, R_CCW_1, static_cast<uint8_t>(R_START | DIR_CW)},
        {R_CW_3, R_CCW_2, R_CW_3, static_cast<uint8_t>(R_START | DIR_CCW)},
        {R_START, R_START, R_START, R_START},
        {R_START, R_START, R_START, R_START},
    }};

    uint8_t clk_dt_pins =
        (gpio_get(ROTARY_CLK) ? 0x2 : 0) | (gpio_get(ROTARY_DT) ? 0x1 : 0);
    encoder_state = TRANSITION_TABLE[encoder_state & STATE_MASK][clk_dt_pins];
    uint8_t direction = encoder_state & DIR_MASK;

    hw_event_t event = {};
    if (direction == DIR_CW) {
        event.type = HW_EVENT_ROTATION;
        event.data.rotation.direction = HW_ROTARY_CW;
        queue_try_add(&event_queue, &event);
    } else if (direction == DIR_CCW) {
        event.type = HW_EVENT_ROTATION;
        event.data.rotation.direction = HW_ROTARY_CCW;
        queue_try_add(&event_queue, &event);
    }
}

void init_button(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
    gpio_set_irq_enabled_with_callback(
        gpio, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &irq_callback);
}

void init_inputs() {
    init_button(GPIO_KEY0);
    init_button(GPIO_KEY1);
    init_button(GPIO_KEY2);
    init_button(GPIO_KEY3);
}

void init_rotary_encoder() {
    gpio_init(ROTARY_CLK);
    gpio_set_dir(ROTARY_CLK, GPIO_IN);
    gpio_disable_pulls(ROTARY_CLK);

    gpio_init(ROTARY_DT);
    gpio_set_dir(ROTARY_DT, GPIO_IN);
    gpio_disable_pulls(ROTARY_DT);

    gpio_set_irq_enabled_with_callback(
        ROTARY_CLK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true,
        &irq_callback);
    gpio_set_irq_enabled_with_callback(
        ROTARY_DT, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true,
        &irq_callback);

    init_button(ROTARY_SW);
    encoder_state = 0;
}

} // namespace

extern "C" void init_hardware() {
    init_event_queue();
    init_inputs();
    init_rotary_encoder();
}

extern "C" bool get_event(hw_event_t *event) {
    return queue_try_remove(&event_queue, event);
}
