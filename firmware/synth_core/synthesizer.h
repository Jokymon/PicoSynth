#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#include <stddef.h>
#include <stdint.h>

namespace Audio {
namespace Synthesizer {

void start();

void note_on(uint8_t channel, uint8_t note, uint8_t velocity);
void note_off(uint8_t channel, uint8_t note, uint8_t velocity);
void render_interleaved_i16(int16_t *samples, size_t frame_count);
void render_packed_i2s16(uint32_t *samples, size_t frame_count);

}
}

#ifdef __cplusplus
extern "C" {
#endif

void synth_audio_start(void);
void synth_audio_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
void synth_audio_note_off(uint8_t channel, uint8_t note, uint8_t velocity);
void synth_audio_render_interleaved_i16(int16_t *samples, size_t frame_count);
void synth_audio_render_packed_i2s16(uint32_t *samples, size_t frame_count);

#ifdef __cplusplus
}
#endif

#endif
