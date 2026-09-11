//! Desktop simulation backend for the RP2040 synthesizer firmware.

use std::sync::{Mutex, OnceLock};

pub const DISPLAY_WIDTH: usize = 128;
pub const DISPLAY_HEIGHT: usize = 128;
pub const DISPLAY_PIXELS: usize = DISPLAY_WIDTH * DISPLAY_HEIGHT;
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

impl Rgb565 {
    pub fn to_xrgb8888(self) -> u32 {
        let red = ((self.0 >> 11) & 0x1f) as u32;
        let green = ((self.0 >> 5) & 0x3f) as u32;
        let blue = (self.0 & 0x1f) as u32;

        let red = red * 255 / 31;
        let green = green * 255 / 63;
        let blue = blue * 255 / 31;

        (red << 16) | (green << 8) | blue
    }
}

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

/// In-memory LCD model for the 1.44 inch ST7735 display.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct SimulatedLcd {
    pixels: [Rgb565; DISPLAY_PIXELS],
    flush_count: u64,
}

impl SimulatedLcd {
    pub fn new() -> Self {
        Self {
            pixels: [Rgb565::default(); DISPLAY_PIXELS],
            flush_count: 0,
        }
    }

    pub fn pixels(&self) -> &[Rgb565; DISPLAY_PIXELS] {
        &self.pixels
    }

    pub fn flush_count(&self) -> u64 {
        self.flush_count
    }
}

impl Default for SimulatedLcd {
    fn default() -> Self {
        Self::new()
    }
}

impl Display for SimulatedLcd {
    fn set_pixel(&mut self, x: usize, y: usize, color: Rgb565) {
        if x < DISPLAY_WIDTH && y < DISPLAY_HEIGHT {
            self.pixels[y * DISPLAY_WIDTH + x] = color;
        }
    }

    fn flush(&mut self) {
        self.flush_count += 1;
    }
}

static LCD: OnceLock<Mutex<SimulatedLcd>> = OnceLock::new();

fn lcd() -> &'static Mutex<SimulatedLcd> {
    LCD.get_or_init(|| Mutex::new(SimulatedLcd::new()))
}

/// Returns a copy of the current simulated LCD framebuffer.
pub fn lcd_framebuffer_snapshot() -> [Rgb565; DISPLAY_PIXELS] {
    lcd().lock().expect("simulated LCD mutex poisoned").pixels
}

pub fn lcd_flush_count() -> u64 {
    lcd()
        .lock()
        .expect("simulated LCD mutex poisoned")
        .flush_count
}

/// Resets the simulated LCD framebuffer to black.
pub fn reset_lcd() {
    *lcd().lock().expect("simulated LCD mutex poisoned") = SimulatedLcd::new();
}

/// Firmware-compatible display entry point.
///
/// C/C++ signature:
///
/// ```c
/// typedef uint16_t UWORD;
/// void LCD_1IN44_Display(UWORD *Image);
/// ```
///
/// The image is expected to contain 128x128 RGB565 pixels in row-major order.
/// A null pointer is ignored.
#[unsafe(no_mangle)]
pub extern "C" fn LCD_1IN44_Display(image: *const u16) {
    if image.is_null() {
        return;
    }

    let image = unsafe { std::slice::from_raw_parts(image, DISPLAY_PIXELS) };
    let mut lcd = lcd().lock().expect("simulated LCD mutex poisoned");

    for (target, source) in lcd.pixels.iter_mut().zip(image.iter()) {
        *target = Rgb565(*source);
    }

    lcd.flush();
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::{Mutex, OnceLock};

    static TEST_LOCK: OnceLock<Mutex<()>> = OnceLock::new();

    fn test_lock() -> std::sync::MutexGuard<'static, ()> {
        TEST_LOCK
            .get_or_init(|| Mutex::new(()))
            .lock()
            .expect("test mutex poisoned")
    }

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

    #[test]
    fn lcd_display_copies_rgb565_framebuffer() {
        let _lock = test_lock();
        reset_lcd();
        let mut image = [0_u16; DISPLAY_PIXELS];
        image[0] = 0xf800;
        image[DISPLAY_WIDTH + 1] = 0x07e0;
        image[DISPLAY_PIXELS - 1] = 0x001f;

        LCD_1IN44_Display(image.as_mut_ptr());

        let snapshot = lcd_framebuffer_snapshot();
        assert_eq!(snapshot[0], Rgb565(0xf800));
        assert_eq!(snapshot[DISPLAY_WIDTH + 1], Rgb565(0x07e0));
        assert_eq!(snapshot[DISPLAY_PIXELS - 1], Rgb565(0x001f));
        assert_eq!(lcd_flush_count(), 1);
    }

    #[test]
    fn lcd_display_ignores_null_pointer() {
        let _lock = test_lock();
        reset_lcd();

        LCD_1IN44_Display(std::ptr::null());

        assert_eq!(lcd_framebuffer_snapshot(), [Rgb565(0); DISPLAY_PIXELS]);
        assert_eq!(lcd_flush_count(), 0);
    }
}
