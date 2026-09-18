#include "synthesizer.h"

#include "midi.h"
#include "voice.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#ifdef PICO_BUILD
#include "audio_pio.h"
#include "pico/multicore.h"
#else
#include <mutex>
#include <queue>
#endif

namespace Audio {
namespace Synthesizer {
namespace {

constexpr uint32_t AUDIO_SAMPLE_FREQ = 40000;
constexpr uint8_t MIDI_NOTE_OFF = 0x8;
constexpr uint8_t MIDI_NOTE_ON = 0x9;
constexpr size_t VOICE_COUNT = 3;

constexpr uint8_t MIDI_MESSAGE_SHIFT = 28;
constexpr uint8_t MIDI_CHANNEL_SHIFT = 24;
constexpr uint8_t MIDI_NOTE_SHIFT = 16;

inline uint32_t encode_midi_message(uint8_t type, uint8_t channel,
                                    uint8_t note, uint8_t velocity) {
    return (static_cast<uint32_t>(type & 0xf) << MIDI_MESSAGE_SHIFT) |
           (static_cast<uint32_t>(channel & 0xf) << MIDI_CHANNEL_SHIFT) |
           (static_cast<uint32_t>(note & 0x7f) << MIDI_NOTE_SHIFT) |
           static_cast<uint32_t>(velocity & 0x7f);
}

inline uint32_t midi_note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
    return encode_midi_message(MIDI_NOTE_OFF, channel, note, velocity);
}

inline uint32_t midi_note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    return encode_midi_message(MIDI_NOTE_ON, channel, note, velocity);
}

std::array<int32_t, 256> sine_samples;
std::array<Voice *, VOICE_COUNT> voices = {};
#ifdef PICO_BUILD
bool core1_started = false;
#else
std::queue<uint32_t> messages;
std::mutex messages_mutex;
bool started = false;
#endif

void create_sinewave_table() {
    for (size_t i = 0; i < sine_samples.size(); i++) {
        int16_t sample = static_cast<int16_t>(
            32767.0 * std::sin(static_cast<double>(i) * 2.0 * 3.14159265358979323846 /
                               static_cast<double>(sine_samples.size())));
        sine_samples[i] = sample;
    }
}

void process_midi_message(uint8_t type, uint8_t channel, uint8_t note,
                          uint8_t velocity) {
    Voice *voice = voices[channel % VOICE_COUNT];
    if (voice == nullptr) {
        return;
    }

    switch (type) {
    case MIDI_NOTE_OFF:
        voice->gate(false);
        break;
    case MIDI_NOTE_ON:
        voice->set_frequency(NOTES[note & 0x7f].frequency);
        voice->gate(velocity > 0);
        break;
    default:
        break;
    }
}

void process_message(uint32_t message) {
    uint8_t type = static_cast<uint8_t>(message >> MIDI_MESSAGE_SHIFT);
    uint8_t channel = static_cast<uint8_t>((message >> MIDI_CHANNEL_SHIFT) & 0xf);
    uint8_t note = static_cast<uint8_t>((message >> MIDI_NOTE_SHIFT) & 0x7f);
    uint8_t velocity = static_cast<uint8_t>(message & 0x7f);
    process_midi_message(type, channel, note, velocity);
}

#ifdef PICO_BUILD
uint32_t next_packed_i2s16_sample() {
    int32_t mixed = 0;
    for (Voice *voice : voices) {
        mixed += voice->sample() / 2;
    }

    if (mixed > INT16_MAX) {
        mixed = INT16_MAX;
    } else if (mixed < INT16_MIN) {
        mixed = INT16_MIN;
    }

    uint16_t sample_bits = static_cast<uint16_t>(static_cast<int16_t>(mixed));
    return (static_cast<uint32_t>(sample_bits) << 16) | sample_bits;
}

void core1_audio_process() {
    audio_pio_init();

    create_sinewave_table();
    for (size_t i = 0; i < voices.size(); i++) {
        voices[i] = new Voice(AUDIO_SAMPLE_FREQ, sine_samples.data());
    }
    voices[1]->set_waveform(Voice::Rectangle);
    voices[2]->set_waveform(Voice::Triangle);

    while (true) {
        while (multicore_fifo_rvalid()) {
            process_message(multicore_fifo_pop_blocking());
        }

        if (!audio_pio_tx_fifo_full()) {
            audio_pio_put_sample(next_packed_i2s16_sample());
        }
    }
}
#else
void drain_messages() {
    std::lock_guard<std::mutex> lock(messages_mutex);

    while (!messages.empty()) {
        uint32_t message = messages.front();
        messages.pop();
        process_message(message);
    }
}

int16_t next_mono_sample() {
    if (!started) {
        start();
    }

    drain_messages();

    int32_t mixed = 0;
    for (Voice *voice : voices) {
        mixed += voice->sample() / 2;
    }

    if (mixed > INT16_MAX) {
        return INT16_MAX;
    }
    if (mixed < INT16_MIN) {
        return INT16_MIN;
    }
    return static_cast<int16_t>(mixed);
}

void push_message(uint32_t message) {
    std::lock_guard<std::mutex> lock(messages_mutex);
    messages.push(message);
}
#endif

} // namespace

void start() {
#ifdef PICO_BUILD
    if (core1_started) {
        return;
    }

    multicore_launch_core1(&core1_audio_process);
    core1_started = true;
#else
    if (started) {
        return;
    }

    create_sinewave_table();
    for (size_t i = 0; i < voices.size(); i++) {
        voices[i] = new Voice(AUDIO_SAMPLE_FREQ, sine_samples.data());
    }
    voices[1]->set_waveform(Voice::Rectangle);
    voices[2]->set_waveform(Voice::Triangle);
    started = true;
#endif
}

void note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    uint32_t message = midi_note_on(channel, note, velocity);
#ifdef PICO_BUILD
    multicore_fifo_push_blocking(message);
#else
    push_message(message);
#endif
}

void note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
    uint32_t message = midi_note_off(channel, note, velocity);
#ifdef PICO_BUILD
    multicore_fifo_push_blocking(message);
#else
    push_message(message);
#endif
}

#ifndef PICO_BUILD
void render_interleaved_i16(int16_t *samples, size_t frame_count) {
    if (samples == nullptr) {
        return;
    }

    for (size_t i = 0; i < frame_count; i++) {
        int16_t sample = next_mono_sample();
        samples[i * 2] = sample;
        samples[i * 2 + 1] = sample;
    }
}

void render_packed_i2s16(uint32_t *samples, size_t frame_count) {
    if (samples == nullptr) {
        return;
    }

    for (size_t i = 0; i < frame_count; i++) {
        uint16_t sample_bits = static_cast<uint16_t>(next_mono_sample());
        samples[i] = (static_cast<uint32_t>(sample_bits) << 16) | sample_bits;
    }
}
#endif

} // namespace Synthesizer
} // namespace Audio

extern "C" void synth_audio_start(void) {
    Audio::Synthesizer::start();
}

extern "C" void synth_audio_note_on(uint8_t channel, uint8_t note,
                                    uint8_t velocity) {
    Audio::Synthesizer::note_on(channel, note, velocity);
}

extern "C" void synth_audio_note_off(uint8_t channel, uint8_t note,
                                     uint8_t velocity) {
    Audio::Synthesizer::note_off(channel, note, velocity);
}

#ifndef PICO_BUILD
extern "C" void synth_audio_render_interleaved_i16(int16_t *samples,
                                                   size_t frame_count) {
    Audio::Synthesizer::render_interleaved_i16(samples, frame_count);
}

extern "C" void synth_audio_render_packed_i2s16(uint32_t *samples,
                                                size_t frame_count) {
    Audio::Synthesizer::render_packed_i2s16(samples, frame_count);
}
#endif
