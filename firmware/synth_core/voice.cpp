#include "voice.h"
#include "math.h"

const double attack = 0.5;   // seconds
const double decay = 0.3;    // seconds
const double sustain = 0.5;  // 50%
const double release = 0.75; // seconds

static inline float calculate_sample_increment(uint32_t tone_frequency_hz, uint32_t sample_frequency_hz)
{
    // 256 samples for a wave
    return (float)(tone_frequency_hz * 256) / (float)(sample_frequency_hz);
}

Voice::Voice(uint32_t sample_frequency_hz, int32_t *sine_samples)
    : state(Voice::Off), sample_frequency_hz(sample_frequency_hz),
      sine_samples(sine_samples),
      wave_form(Voice::Sine),
      frequency_hz(440),
      hull_value(0.0),
      current_index(0.0),
      sample_increment(0.0)
{
}

void Voice::set_frequency(uint32_t frequency_hz)
{
    this->frequency_hz = frequency_hz;
    sample_increment = calculate_sample_increment(frequency_hz, sample_frequency_hz);
}

int16_t Voice::sample()
{
    current_index += sample_increment;
    if (current_index > 256.0)
    {
        current_index -= 256.0;
    }

    if (state != Voice::Off)
    {
        switch (state) {
            case Voice::Attack:
                hull_value += 1/(attack * sample_frequency_hz);
                if (hull_value>=1.0) {
                    hull_value = 1.0;
                    state = Voice::Decay;
                }
                break;
            case Voice::Decay:
                hull_value -= (1-sustain)/(decay * sample_frequency_hz);
                if (hull_value<=sustain) {
                    hull_value = sustain;
                    state = Voice::Sustain;
                }
                break;
            case Voice::Release:
                hull_value -= sustain/(release * sample_frequency_hz);
                if (hull_value<=0.0) {
                    hull_value = 0.0;
                    state = Voice::Off;
                }
                break;
        }
        return static_cast<int16_t>(
            static_cast<double>(sine_samples[(size_t)current_index]) *
            hull_value);
    }
    else
    {
        return 0;
    }
}
