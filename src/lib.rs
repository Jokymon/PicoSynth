//! Core simulation interfaces for an RP2040-based synthesizer/sequencer.
//!
//! The first goal of this crate is not cycle-accurate RP2040 emulation. It is
//! a stable boundary between synthesizer logic and the outside world: audio
//! samples, display pixels, buttons, encoder movement, and simulated time.

pub const DISPLAY_WIDTH: usize = 128;
pub const DISPLAY_HEIGHT: usize = 128;
pub const DEFAULT_SAMPLE_RATE_HZ: u32 = 44_100;

/// A signed stereo audio frame after the synthesizer has produced a sample.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct StereoSample {
    pub left: i16,
    pub right: i16,
}

impl StereoSample {
    pub const SILENCE: Self = Self { left: 0, right: 0 };
}

/// RGB565 color as used by many ST7735 display configurations.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Rgb565(pub u16);

/// Physical controls available on the front panel.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Control {
    Button(usize),
    EncoderButton,
}

/// Input events produced by GPIO-backed controls.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum InputEvent {
    Pressed(Control),
    Released(Control),
    EncoderDelta(i32),
}

/// Host-side sink for generated audio.
pub trait AudioSink {
    fn sample_rate_hz(&self) -> u32;
    fn push_sample(&mut self, sample: StereoSample);
}

/// Host-side display target for the simulated ST7735 framebuffer.
pub trait Display {
    fn set_pixel(&mut self, x: usize, y: usize, color: Rgb565);
    fn flush(&mut self);
}

/// Source of front-panel input events.
pub trait InputSource {
    fn poll_event(&mut self) -> Option<InputEvent>;
}

/// Minimal machine that can be stepped by a host simulator.
pub trait SimulatedDevice {
    fn step(&mut self, host: &mut dyn HardwareHost);
}

/// Hardware services exposed to portable firmware or a simulator adapter.
pub trait HardwareHost {
    fn audio(&mut self) -> &mut dyn AudioSink;
    fn display(&mut self) -> &mut dyn Display;
    fn input(&mut self) -> &mut dyn InputSource;
    fn micros(&self) -> u64;
}

/// In-memory audio sink useful for deterministic tests.
#[derive(Debug)]
pub struct BufferedAudio {
    sample_rate_hz: u32,
    samples: Vec<StereoSample>,
}

impl BufferedAudio {
    pub fn new(sample_rate_hz: u32) -> Self {
        Self {
            sample_rate_hz,
            samples: Vec::new(),
        }
    }

    pub fn samples(&self) -> &[StereoSample] {
        &self.samples
    }
}

impl Default for BufferedAudio {
    fn default() -> Self {
        Self::new(DEFAULT_SAMPLE_RATE_HZ)
    }
}

impl AudioSink for BufferedAudio {
    fn sample_rate_hz(&self) -> u32 {
        self.sample_rate_hz
    }

    fn push_sample(&mut self, sample: StereoSample) {
        self.samples.push(sample);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn buffered_audio_keeps_sample_order() {
        let mut audio = BufferedAudio::new(DEFAULT_SAMPLE_RATE_HZ);

        audio.push_sample(StereoSample { left: 1, right: 2 });
        audio.push_sample(StereoSample { left: 3, right: 4 });

        assert_eq!(audio.sample_rate_hz(), DEFAULT_SAMPLE_RATE_HZ);
        assert_eq!(
            audio.samples(),
            &[
                StereoSample { left: 1, right: 2 },
                StereoSample { left: 3, right: 4 }
            ]
        );
    }
}
