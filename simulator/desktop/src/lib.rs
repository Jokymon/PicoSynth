//! Desktop simulation backend for the RP2040 synthesizer firmware.

use std::collections::VecDeque;
use std::ffi::c_int;
use std::sync::{Mutex, OnceLock};

pub const DISPLAY_WIDTH: usize = 128;
pub const DISPLAY_HEIGHT: usize = 128;
pub const DISPLAY_PIXELS: usize = DISPLAY_WIDTH * DISPLAY_HEIGHT;
pub const DEFAULT_SAMPLE_RATE_HZ: u32 = 44_100;

pub const HW_KEY_ROTARY_SWITCH: c_int = 0;
pub const HW_KEY_BUTTON_1: c_int = 1;
pub const HW_KEY_BUTTON_2: c_int = 2;
pub const HW_KEY_BUTTON_3: c_int = 3;
pub const HW_KEY_BUTTON_4: c_int = 4;

pub const HW_ROTARY_CW: c_int = 0;
pub const HW_ROTARY_CCW: c_int = 1;

pub const HW_KEY_RELEASED: c_int = 0;
pub const HW_KEY_PRESSED: c_int = 1;

pub const HW_EVENT_ROTATION: c_int = 0;
pub const HW_EVENT_KEY: c_int = 1;

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

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct HwRotationEvent {
    pub direction: c_int,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct HwKeyEvent {
    pub key: c_int,
    pub state: c_int,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub union HwEventData {
    pub rotation: HwRotationEvent,
    pub key: HwKeyEvent,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct HwEvent {
    pub event_type: c_int,
    pub data: HwEventData,
}

impl HwEvent {
    pub fn key(key: c_int, state: c_int) -> Self {
        Self {
            event_type: HW_EVENT_KEY,
            data: HwEventData {
                key: HwKeyEvent { key, state },
            },
        }
    }

    pub fn rotation(direction: c_int) -> Self {
        Self {
            event_type: HW_EVENT_ROTATION,
            data: HwEventData {
                rotation: HwRotationEvent { direction },
            },
        }
    }
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
static EVENTS: OnceLock<Mutex<VecDeque<HwEvent>>> = OnceLock::new();

fn lcd() -> &'static Mutex<SimulatedLcd> {
    LCD.get_or_init(|| Mutex::new(SimulatedLcd::new()))
}

fn events() -> &'static Mutex<VecDeque<HwEvent>> {
    EVENTS.get_or_init(|| Mutex::new(VecDeque::new()))
}

/// Returns a copy of the current simulated LCD framebuffer.
pub fn lcd_framebuffer_snapshot() -> [Rgb565; DISPLAY_PIXELS] {
    lcd().lock().expect("simulated LCD mutex poisoned").pixels
}

pub fn lcd_window_frame_snapshot() -> [u32; DISPLAY_PIXELS] {
    let pixels = lcd_framebuffer_snapshot();
    let mut frame = [0_u32; DISPLAY_PIXELS];

    for y in 0..DISPLAY_HEIGHT {
        for x in 0..DISPLAY_WIDTH {
            let source = pixels[(DISPLAY_HEIGHT - 1 - x) * DISPLAY_WIDTH + y];
            frame[y * DISPLAY_WIDTH + x] = source.to_xrgb8888();
        }
    }

    frame
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

pub fn push_key_event(key: c_int, state: c_int) {
    events()
        .lock()
        .expect("simulator event queue mutex poisoned")
        .push_back(HwEvent::key(key, state));
}

pub fn push_rotation_event(direction: c_int) {
    events()
        .lock()
        .expect("simulator event queue mutex poisoned")
        .push_back(HwEvent::rotation(direction));
}

#[unsafe(no_mangle)]
pub extern "C" fn init_hardware() {
    events()
        .lock()
        .expect("simulator event queue mutex poisoned")
        .clear();
}

#[unsafe(no_mangle)]
pub extern "C" fn get_event(event: *mut HwEvent) -> bool {
    if event.is_null() {
        return false;
    }

    let Some(next_event) = events()
        .lock()
        .expect("simulator event queue mutex poisoned")
        .pop_front()
    else {
        return false;
    };

    unsafe {
        *event = next_event;
    }
    true
}

#[unsafe(no_mangle)]
pub extern "C" fn synth_simulator_push_key_event(key: c_int, state: c_int) {
    push_key_event(key, state);
}

#[unsafe(no_mangle)]
pub extern "C" fn synth_simulator_push_rotation_event(direction: c_int) {
    push_rotation_event(direction);
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

    let image = unsafe { std::slice::from_raw_parts(image.cast::<u8>(), DISPLAY_PIXELS * 2) };
    let mut lcd = lcd().lock().expect("simulated LCD mutex poisoned");

    for (target, source) in lcd.pixels.iter_mut().zip(image.chunks_exact(2)) {
        *target = Rgb565(u16::from_be_bytes([source[0], source[1]]));
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
            .unwrap_or_else(|poisoned| poisoned.into_inner())
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
        let mut image = [0_u8; DISPLAY_PIXELS * 2];
        image[0..2].copy_from_slice(&0xf800_u16.to_be_bytes());
        let green = (DISPLAY_WIDTH + 1) * 2;
        image[green..green + 2].copy_from_slice(&0x07e0_u16.to_be_bytes());
        let blue = (DISPLAY_PIXELS - 1) * 2;
        image[blue..blue + 2].copy_from_slice(&0x001f_u16.to_be_bytes());

        LCD_1IN44_Display(image.as_mut_ptr().cast::<u16>());

        let snapshot = lcd_framebuffer_snapshot();
        assert_eq!(snapshot[0], Rgb565(0xf800));
        assert_eq!(snapshot[DISPLAY_WIDTH + 1], Rgb565(0x07e0));
        assert_eq!(snapshot[DISPLAY_PIXELS - 1], Rgb565(0x001f));
        assert_eq!(lcd_flush_count(), 1);
    }

    #[test]
    fn simulator_event_queue_returns_events_in_order() {
        let _lock = test_lock();
        init_hardware();

        push_key_event(HW_KEY_BUTTON_1, HW_KEY_PRESSED);
        push_rotation_event(HW_ROTARY_CW);

        let mut event = HwEvent::key(0, 0);

        assert!(get_event(&mut event));
        assert_eq!(event.event_type, HW_EVENT_KEY);
        unsafe {
            assert_eq!(event.data.key.key, HW_KEY_BUTTON_1);
            assert_eq!(event.data.key.state, HW_KEY_PRESSED);
        }

        assert!(get_event(&mut event));
        assert_eq!(event.event_type, HW_EVENT_ROTATION);
        unsafe {
            assert_eq!(event.data.rotation.direction, HW_ROTARY_CW);
        }

        assert!(!get_event(&mut event));
    }

    #[test]
    fn synth_audio_renders_samples_after_note_on() {
        unsafe extern "C" {
            fn synth_audio_start();
            fn synth_audio_note_on(channel: u8, note: u8, velocity: u8);
            fn synth_audio_note_off(channel: u8, note: u8, velocity: u8);
            fn synth_audio_render_interleaved_i16(samples: *mut i16, frame_count: usize);
        }

        let _lock = test_lock();
        let mut samples = [0_i16; 4096 * 2];

        unsafe {
            synth_audio_start();
            synth_audio_note_on(0, 69, 64);
            synth_audio_render_interleaved_i16(samples.as_mut_ptr(), 4096);
            synth_audio_note_off(0, 69, 64);
        }

        assert!(samples.iter().any(|sample| *sample != 0));
    }

    #[test]
    fn synth_audio_attack_preserves_signed_waveform() {
        unsafe extern "C" {
            fn synth_audio_start();
            fn synth_audio_note_on(channel: u8, note: u8, velocity: u8);
            fn synth_audio_note_off(channel: u8, note: u8, velocity: u8);
            fn synth_audio_render_interleaved_i16(samples: *mut i16, frame_count: usize);
        }

        let _lock = test_lock();
        let mut samples = [0_i16; 2048 * 2];

        unsafe {
            synth_audio_start();
            synth_audio_note_on(0, 69, 64);
            synth_audio_render_interleaved_i16(samples.as_mut_ptr(), 2048);
            synth_audio_note_off(0, 69, 64);
        }

        let left_channel = samples.iter().step_by(2);
        assert!(left_channel.clone().any(|sample| *sample > 0));
        assert!(left_channel.clone().any(|sample| *sample < 0));
    }

    #[test]
    fn packed_i2s_render_preserves_twos_complement_sample_bits() {
        unsafe extern "C" {
            fn synth_audio_start();
            fn synth_audio_note_on(channel: u8, note: u8, velocity: u8);
            fn synth_audio_note_off(channel: u8, note: u8, velocity: u8);
            fn synth_audio_render_packed_i2s16(samples: *mut u32, frame_count: usize);
        }

        let _lock = test_lock();
        let mut packed_samples = [0_u32; 4096];

        unsafe {
            synth_audio_start();
            synth_audio_note_on(0, 69, 64);
            synth_audio_render_packed_i2s16(packed_samples.as_mut_ptr(), 4096);
            synth_audio_note_off(0, 69, 64);
        }

        assert!(packed_samples.iter().any(|word| {
            let left = (word >> 16) as u16;
            let right = *word as u16;
            left == right && (left & 0x8000) != 0
        }));
    }

    #[test]
    fn lcd_display_ignores_null_pointer() {
        let _lock = test_lock();
        reset_lcd();

        LCD_1IN44_Display(std::ptr::null());

        assert_eq!(lcd_framebuffer_snapshot(), [Rgb565(0); DISPLAY_PIXELS]);
        assert_eq!(lcd_flush_count(), 0);
    }

    #[test]
    fn simulator_startup_draws_lcd_frame() {
        unsafe extern "C" {
            fn synth_simulator_main_once() -> i32;
        }

        let _lock = test_lock();
        reset_lcd();

        let result = unsafe { synth_simulator_main_once() };

        assert_eq!(result, 0);
        assert_eq!(lcd_flush_count(), 1);
        assert!(
            lcd_framebuffer_snapshot()
                .iter()
                .any(|pixel| *pixel != Rgb565(0x0000))
        );
    }

    #[test]
    fn lcd_window_frame_rotates_raw_framebuffer_clockwise() {
        let _lock = test_lock();
        reset_lcd();

        let mut image = [0_u8; DISPLAY_PIXELS * 2];
        image[0..2].copy_from_slice(&0xf800_u16.to_be_bytes());
        let top_right = (DISPLAY_WIDTH - 1) * 2;
        image[top_right..top_right + 2].copy_from_slice(&0x07e0_u16.to_be_bytes());
        let bottom_left = (DISPLAY_PIXELS - DISPLAY_WIDTH) * 2;
        image[bottom_left..bottom_left + 2].copy_from_slice(&0x001f_u16.to_be_bytes());

        LCD_1IN44_Display(image.as_mut_ptr().cast::<u16>());

        let frame = lcd_window_frame_snapshot();
        assert_eq!(frame[DISPLAY_WIDTH - 1], Rgb565(0xf800).to_xrgb8888());
        assert_eq!(frame[DISPLAY_PIXELS - 1], Rgb565(0x07e0).to_xrgb8888());
        assert_eq!(frame[0], Rgb565(0x001f).to_xrgb8888());
    }
}
