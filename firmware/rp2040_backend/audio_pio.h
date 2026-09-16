#ifndef PICO_SYNTH_AUDIO_PIO_H
#define PICO_SYNTH_AUDIO_PIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PICO_SYNTH_AUDIO_FREQ 40000u
#define PICO_SYNTH_AUDIO_DATA_PIN 26u
#define PICO_SYNTH_AUDIO_CLOCK_PIN_BASE 27u
#define PICO_SYNTH_AUDIO_SM 0u

void audio_pio_init(void);
bool audio_pio_tx_fifo_full(void);
void audio_pio_put_sample(uint32_t sample);

#ifdef __cplusplus
}
#endif

#endif
