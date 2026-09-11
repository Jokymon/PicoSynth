#ifndef PICO_SYNTH_FIRMWARE_LCD_1IN44_H
#define PICO_SYNTH_FIRMWARE_LCD_1IN44_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t UWORD;

void LCD_1IN44_Display(UWORD *Image);

#ifdef __cplusplus
}
#endif

#endif
