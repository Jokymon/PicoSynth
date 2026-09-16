#include "audio_pio.h"

#include "audio_pio.pio.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#define AUDIO_PIO pio0
#define GPIO_FUNC_AUDIO_PIO GPIO_FUNC_PIO0

void audio_pio_init(void) {
    gpio_set_function(PICO_SYNTH_AUDIO_DATA_PIN, GPIO_FUNC_AUDIO_PIO);
    gpio_set_function(PICO_SYNTH_AUDIO_CLOCK_PIN_BASE, GPIO_FUNC_AUDIO_PIO);
    gpio_set_function(PICO_SYNTH_AUDIO_CLOCK_PIN_BASE + 1, GPIO_FUNC_AUDIO_PIO);

    PIO pio = AUDIO_PIO;
    uint sm = PICO_SYNTH_AUDIO_SM;

    pio_sm_claim(pio, sm);
    uint offset = pio_add_program(pio, &audio_pio_program);
    audio_pio_program_init(
        pio, sm, offset, PICO_SYNTH_AUDIO_DATA_PIN,
        PICO_SYNTH_AUDIO_CLOCK_PIN_BASE);

    uint32_t system_clock_frequency = clock_get_hz(clk_sys);
    uint32_t divider = system_clock_frequency * 4 / PICO_SYNTH_AUDIO_FREQ;
    pio_sm_set_clkdiv_int_frac(pio, sm, divider >> 8u, divider & 0xffu);

    pio_sm_set_enabled(pio, sm, true);
}

bool audio_pio_tx_fifo_full(void) {
    return pio_sm_is_tx_fifo_full(AUDIO_PIO, PICO_SYNTH_AUDIO_SM);
}

void audio_pio_put_sample(uint32_t sample) {
    pio_sm_put(AUDIO_PIO, PICO_SYNTH_AUDIO_SM, sample);
}
