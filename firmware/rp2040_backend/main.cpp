#include "lcd_1in44.h"
#include "synth_c_api.h"

#include <cstdlib>

int main() {
    const UDOUBLE IMAGESIZE = LCD_1IN44_HEIGHT * LCD_1IN44_WIDTH * 2;
    screen = (UWORD *)malloc(IMAGESIZE);

    if (screen == nullptr) {
        return 1;
    }

    // RP2040 Pico SDK initialization will live here:
    // - clocks
    // - PIO I2S audio
    // - SPI ST7735 display
    // - GPIO buttons and encoder
    // - multicore launch
    synth_app_start(screen);

    while (true) {
        synth_app_loop(screen);
    }

    return 0;
}
