#include "synth_c_api.h"

#include <cstdint>

namespace {

UWORD rgb565(std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
    return static_cast<UWORD>(((red & 0xf8) << 8) | ((green & 0xfc) << 3) |
                              (blue >> 3));
}

} // namespace

extern "C" void synth_demo_render_frame(UWORD *screen) {
    if (screen == nullptr) {
        return;
    }

    for (int y = 0; y < LCD_1IN44_HEIGHT; ++y) {
        for (int x = 0; x < LCD_1IN44_WIDTH; ++x) {
            const auto red = static_cast<std::uint8_t>(x * 2);
            const auto green = static_cast<std::uint8_t>(y * 2);
            const auto blue = static_cast<std::uint8_t>((x ^ y) * 2);
            screen[y * LCD_1IN44_WIDTH + x] = rgb565(red, green, blue);
        }
    }

    for (int x = 0; x < LCD_1IN44_WIDTH; ++x) {
        screen[(LCD_1IN44_HEIGHT / 2) * LCD_1IN44_WIDTH + x] =
            rgb565(255, 255, 255);
    }

    for (int y = 0; y < LCD_1IN44_HEIGHT; ++y) {
        screen[y * LCD_1IN44_WIDTH + (LCD_1IN44_WIDTH / 2)] =
            rgb565(255, 255, 255);
    }

    LCD_1IN44_Display(screen);
}
