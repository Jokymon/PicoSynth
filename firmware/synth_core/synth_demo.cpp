#include "synth_c_api.h"

#include "lcd_1in44.h"

#include <cstdint>

namespace {

constexpr int kDisplayWidth = 128;
constexpr int kDisplayHeight = 128;
constexpr int kDisplayPixels = kDisplayWidth * kDisplayHeight;

UWORD rgb565(std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
    return static_cast<UWORD>(((red & 0xf8) << 8) | ((green & 0xfc) << 3) |
                              (blue >> 3));
}

} // namespace

extern "C" void synth_demo_render_frame(void) {
    static UWORD image[kDisplayPixels];

    for (int y = 0; y < kDisplayHeight; ++y) {
        for (int x = 0; x < kDisplayWidth; ++x) {
            const auto red = static_cast<std::uint8_t>(x * 2);
            const auto green = static_cast<std::uint8_t>(y * 2);
            const auto blue = static_cast<std::uint8_t>((x ^ y) * 2);
            image[y * kDisplayWidth + x] = rgb565(red, green, blue);
        }
    }

    for (int x = 0; x < kDisplayWidth; ++x) {
        image[(kDisplayHeight / 2) * kDisplayWidth + x] = rgb565(255, 255, 255);
    }

    for (int y = 0; y < kDisplayHeight; ++y) {
        image[y * kDisplayWidth + (kDisplayWidth / 2)] = rgb565(255, 255, 255);
    }

    LCD_1IN44_Display(image);
}
