#ifndef PICO_SYNTH_FIRMWARE_LCD_1IN44_H
#define PICO_SYNTH_FIRMWARE_LCD_1IN44_H

#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_1IN44_HEIGHT 128
#define LCD_1IN44_WIDTH 128

typedef struct {
    UWORD WIDTH;
    UWORD HEIGHT;
} LCD_1IN44_ATTRIBUTES;

extern const LCD_1IN44_ATTRIBUTES LCD_1IN44;

void LCD_1IN44_Display(UWORD *Image);

#ifdef __cplusplus
}
#endif

#endif
