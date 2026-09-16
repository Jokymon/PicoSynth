#include "lcd_1in44.h"
#include "synth_c_api.h"
#include "tusb.h"
#include "DEV_Config.h"

#include <cstdlib>

int main()
{
    stdio_init_all();

    while (!tud_cdc_connected()) {
        printf(".");
        sleep_ms(100);
    }

    if (DEV_Module_Init() != 0)
    {
        return -1;
    }

    const UDOUBLE IMAGESIZE = LCD_1IN44_HEIGHT * LCD_1IN44_WIDTH * 2;
    UWORD *screen = (UWORD *)malloc(IMAGESIZE);
    if (screen == nullptr) {
        printf("Failed to allocate screen buffer\r\n");
        return -1;
    }

    synth_app_start(screen);

    while (true) {
        synth_app_loop(screen);
    }

    return 0;
}
