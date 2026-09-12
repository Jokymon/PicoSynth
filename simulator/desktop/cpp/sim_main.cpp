#include "lcd_1in44.h"
#include "synth_c_api.h"

#include <cstdlib>

namespace {

UWORD *screen = nullptr;

} // namespace

extern "C" int synth_simulator_main_once(void) {
    const UDOUBLE IMAGESIZE = LCD_1IN44_HEIGHT * LCD_1IN44_WIDTH * 2;
    screen = (UWORD *)malloc(IMAGESIZE);

    if (screen == nullptr) {
        return 1;
    }

    synth_app_start(screen);
    return 0;
}

extern "C" void synth_simulator_loop_once(void) {
    synth_app_loop(screen);
}
