#include "synthesizer.h"

#include "midi.h"
#include "voice.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <queue>

namespace Audio {
namespace Synthesizer {
namespace {

constexpr uint32_t PICO_AUDIO_FREQ = 40000;
constexpr uint8_t MIDI_NOTE_OFF = 0x8;
constexpr uint8_t MIDI_NOTE_ON = 0x9;
constexpr size_t VOICE_COUNT = 2;

struct Message {
    uint8_t type;
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;
};

std::array<int32_t, 256> sine_samples;
std::array<Voice *, VOICE_COUNT> voices = {};
std::queue<Message> messages;
std::mutex messages_mutex;
bool started = false;

void create_sinewave_table() {
    for (size_t i = 0; i < sine_samples.size(); i++) {
        int16_t sample = static_cast<int16_t>(
            32767.0 * std::sin(static_cast<double>(i) * 2.0 * 3.14159265358979323846 /
                               static_cast<double>(sine_samples.size())));
        sine_samples[i] = sample;
    }
}

void process_message(const Message &message) {
    size_t channel = message.channel % VOICE_COUNT;

    switch (message.type) {
    case MIDI_NOTE_OFF:
        voices[channel]->gate(false);
        break;
    case MIDI_NOTE_ON:
        voices[channel]->set_frequency(NOTES[message.note & 0x7f].frequency);
        voices[channel]->gate(message.velocity > 0);
        break;
    default:
        break;
    }
}

void drain_messages() {
    std::lock_guard<std::mutex> lock(messages_mutex);

    while (!messages.empty()) {
        process_message(messages.front());
        messages.pop();
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

void push_message(uint8_t type, uint8_t channel, uint8_t note, uint8_t velocity) {
    std::lock_guard<std::mutex> lock(messages_mutex);
    messages.push(Message{type, channel, note, velocity});
}

} // namespace

void start() {
    if (started) {
        return;
    }

    create_sinewave_table();
    for (size_t i = 0; i < voices.size(); i++) {
        voices[i] = new Voice(PICO_AUDIO_FREQ, sine_samples.data());
    }
    started = true;
}

void note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    push_message(MIDI_NOTE_ON, channel, note, velocity);
}

void note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
    push_message(MIDI_NOTE_OFF, channel, note, velocity);
}

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

extern "C" void synth_audio_render_interleaved_i16(int16_t *samples,
                                                   size_t frame_count) {
    Audio::Synthesizer::render_interleaved_i16(samples, frame_count);
}

extern "C" void synth_audio_render_packed_i2s16(uint32_t *samples,
                                                size_t frame_count) {
    Audio::Synthesizer::render_packed_i2s16(samples, frame_count);
}
