/*****************************************************************************
* | File      	:   DEV_Config.c
* | Author      :   
* | Function    :   Hardware underlying interface
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-03-16
* | Info        :   
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of theex Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
******************************************************************************/
#include "DEV_Config.h"

#define SPI_PORT spi1

uint slice_num;

/**
 * SPI
**/
void DEV_SPI_WriteByte(uint8_t Value)
{
    spi_write_blocking(SPI_PORT, &Value, 1);
}

void DEV_SPI_Write_nByte(uint8_t pData[], uint32_t Len)
{
    spi_write_blocking(SPI_PORT, pData, Len);
}

void DEV_GPIO_Init(void)
{
    gpio_init(LCD_RST_PIN);
    gpio_set_dir(LCD_RST_PIN, GPIO_OUT);
    gpio_init(LCD_DC_PIN);
    gpio_set_dir(LCD_DC_PIN, GPIO_OUT);
    gpio_init(LCD_CS_PIN);
    gpio_set_dir(LCD_CS_PIN, GPIO_OUT);
    gpio_init(LCD_BL_PIN);
    gpio_set_dir(LCD_BL_PIN, GPIO_OUT);
    
    gpio_init(LCD_CS_PIN);
    gpio_set_dir(LCD_CS_PIN, GPIO_OUT);
    gpio_init(LCD_BL_PIN);
    gpio_set_dir(LCD_BL_PIN, GPIO_OUT);

    gpio_put(LCD_CS_PIN, 1);
    gpio_put(LCD_DC_PIN, 0);
    gpio_put(LCD_BL_PIN, 1);
}

/******************************************************************************
function:	Module Initialize, the library and initialize the pins, SPI protocol
parameter:
Info:
******************************************************************************/
UBYTE DEV_Module_Init(void)
{
    // SPI Config
    spi_init(SPI_PORT, 10000 * 1000);
    gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
    
    // GPIO Config
    DEV_GPIO_Init();
    
    // PWM Config
    gpio_set_function(LCD_BL_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(LCD_BL_PIN);
    pwm_set_wrap(slice_num, 100);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1);
    pwm_set_clkdiv(slice_num,50);
    pwm_set_enabled(slice_num, true);
    
    printf("DEV_Module_Init OK \r\n");
    return 0;
}

void DEV_SET_PWM(uint8_t Value){
    if(Value<0 || Value >100){
        printf("DEV_SET_PWM Error \r\n");
    }else {
        pwm_set_chan_level(slice_num, PWM_CHAN_B, Value);
    }
}
