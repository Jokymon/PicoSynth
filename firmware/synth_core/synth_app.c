#include "synth_c_api.h"

#include "GUI_Paint.h"

void synth_app_start(UWORD *screen) {
    if (screen == 0) {
        return;
    }

    Paint_NewImage((UBYTE *)screen, LCD_1IN44.WIDTH, LCD_1IN44.HEIGHT,
                   ROTATE_270, BLACK);

    Paint_SetScale(65);
    Paint_SetRotate(ROTATE_270);
    Paint_Clear(BLACK);

    Paint_DrawString_EN(0, 40, "Synth", &Font12, WHITE, BLACK);
    LCD_1IN44_Display(screen);
}
