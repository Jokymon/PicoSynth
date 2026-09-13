#ifndef VOICE_H
#define VOICE_H

#include <stdint.h>
#include <stdio.h>

class Voice
{
public:
    enum WaveForm
    {
        Sine
    };

    explicit Voice(uint32_t sample_frequency_hz, int32_t *sine_samples);

    void set_frequency(uint32_t frequency_hz);
    
    // Turn the output of this voice on or off; in case of no ADSR envelope
    // this will directly turn the wave on or off. In case of ADSR, gating
    // on will start the attack phase and go through decay to stain on sustain
    // while gating off will start the decay phase.
    inline void gate(bool on)
    {
        if (on)
        {
            state = Voice::Attack;
        }
        else
        {
            hull_value = 0.0;
            state = Voice::Off;
        }
    }

    // Get the next audio sample for the initially configured sample frequency
    int16_t sample();

private:
    enum State
    {
        On,
        Off,
        Attack,
        Decay,
        Sustain,
        Release
    };

    State state;
    uint32_t sample_frequency_hz;
    int32_t *sine_samples;
    WaveForm wave_form;
    uint32_t frequency_hz;
    double hull_value;

    // Numerically controlled oscillator
    float current_index;
    float sample_increment;
};

#endif
