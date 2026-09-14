use std::ffi::{c_char, c_void};
use std::mem::size_of;
use std::ptr::null_mut;
use std::thread;
use std::time::Duration;

const SAMPLE_RATE_HZ: u32 = 40_000;
const CHANNELS: u16 = 2;
const BITS_PER_SAMPLE: u16 = 16;
const BUFFER_FRAMES: usize = 512;
const BUFFER_COUNT: usize = 4;

const CALLBACK_NULL: u32 = 0;
const WAVE_FORMAT_PCM: u16 = 1;
const WAVE_MAPPER: u32 = u32::MAX;
const MMSYSERR_NOERROR: u32 = 0;
const WHDR_DONE: u32 = 0x0000_0001;
const MAXPNAMELEN: usize = 32;

type HWaveOut = *mut c_void;

#[derive(Clone, Debug)]
pub struct AudioDevice {
    pub id: u32,
    pub name: String,
}

#[repr(C)]
struct WaveFormatEx {
    format_tag: u16,
    channels: u16,
    samples_per_sec: u32,
    avg_bytes_per_sec: u32,
    block_align: u16,
    bits_per_sample: u16,
    cb_size: u16,
}

#[repr(C)]
struct WaveHdr {
    data: *mut c_char,
    buffer_length: u32,
    bytes_recorded: u32,
    user: usize,
    flags: u32,
    loops: u32,
    next: *mut WaveHdr,
    reserved: usize,
}

#[repr(C)]
struct WaveOutCapsW {
    mid: u16,
    pid: u16,
    driver_version: u32,
    pname: [u16; MAXPNAMELEN],
    formats: u32,
    channels: u16,
    reserved1: u16,
    support: u32,
}

struct AudioBuffer {
    samples: Box<[i16]>,
    header: Box<WaveHdr>,
}

unsafe extern "system" {
    fn waveOutGetNumDevs() -> u32;
    fn waveOutGetDevCapsW(device_id: usize, caps: *mut WaveOutCapsW, caps_size: u32) -> u32;
    fn waveOutOpen(
        wave_out: *mut HWaveOut,
        device_id: u32,
        format: *const WaveFormatEx,
        callback: usize,
        instance: usize,
        flags: u32,
    ) -> u32;
    fn waveOutPrepareHeader(wave_out: HWaveOut, header: *mut WaveHdr, size: u32) -> u32;
    fn waveOutWrite(wave_out: HWaveOut, header: *mut WaveHdr, size: u32) -> u32;
    fn waveOutClose(wave_out: HWaveOut) -> u32;
}

unsafe extern "C" {
    fn synth_audio_render_interleaved_i16(samples: *mut i16, frame_count: usize);
}

pub fn audio_devices() -> Result<Vec<AudioDevice>, String> {
    let device_count = unsafe { waveOutGetNumDevs() };
    let mut devices = Vec::new();

    for id in 0..device_count {
        let mut caps = WaveOutCapsW {
            mid: 0,
            pid: 0,
            driver_version: 0,
            pname: [0; MAXPNAMELEN],
            formats: 0,
            channels: 0,
            reserved1: 0,
            support: 0,
        };
        let result =
            unsafe { waveOutGetDevCapsW(id as usize, &mut caps, size_of::<WaveOutCapsW>() as u32) };
        if result != MMSYSERR_NOERROR {
            return Err(format!("waveOutGetDevCapsW failed with code {result}"));
        }

        let name_len = caps
            .pname
            .iter()
            .position(|character| *character == 0)
            .unwrap_or(caps.pname.len());
        let name = String::from_utf16_lossy(&caps.pname[..name_len]);
        devices.push(AudioDevice { id, name });
    }

    Ok(devices)
}

pub fn start_audio_thread(device_id: Option<u32>) {
    thread::spawn(move || {
        if let Err(error) = run_audio(device_id.unwrap_or(WAVE_MAPPER)) {
            eprintln!("audio output stopped: {error}");
        }
    });
}

fn run_audio(device_id: u32) -> Result<(), String> {
    let block_align = CHANNELS * BITS_PER_SAMPLE / 8;
    let format = WaveFormatEx {
        format_tag: WAVE_FORMAT_PCM,
        channels: CHANNELS,
        samples_per_sec: SAMPLE_RATE_HZ,
        avg_bytes_per_sec: SAMPLE_RATE_HZ * u32::from(block_align),
        block_align,
        bits_per_sample: BITS_PER_SAMPLE,
        cb_size: 0,
    };

    let mut wave_out: HWaveOut = null_mut();
    let result = unsafe { waveOutOpen(&mut wave_out, device_id, &format, 0, 0, CALLBACK_NULL) };
    if result != MMSYSERR_NOERROR {
        return Err(format!("waveOutOpen failed with code {result}"));
    }

    let mut buffers = create_buffers();
    let header_size = size_of::<WaveHdr>() as u32;

    for buffer in &mut buffers {
        render_buffer(buffer);
        let result = unsafe { waveOutPrepareHeader(wave_out, &mut *buffer.header, header_size) };
        if result != MMSYSERR_NOERROR {
            unsafe {
                waveOutClose(wave_out);
            }
            return Err(format!("waveOutPrepareHeader failed with code {result}"));
        }

        let result = unsafe { waveOutWrite(wave_out, &mut *buffer.header, header_size) };
        if result != MMSYSERR_NOERROR {
            unsafe {
                waveOutClose(wave_out);
            }
            return Err(format!("waveOutWrite failed with code {result}"));
        }
    }

    loop {
        for buffer in &mut buffers {
            if buffer.header.flags & WHDR_DONE == 0 {
                continue;
            }

            render_buffer(buffer);
            let result = unsafe { waveOutWrite(wave_out, &mut *buffer.header, header_size) };
            if result != MMSYSERR_NOERROR {
                unsafe {
                    waveOutClose(wave_out);
                }
                return Err(format!("waveOutWrite failed with code {result}"));
            }
        }

        thread::sleep(Duration::from_millis(1));
    }
}

fn create_buffers() -> Vec<AudioBuffer> {
    (0..BUFFER_COUNT)
        .map(|_| {
            let mut samples = vec![0_i16; BUFFER_FRAMES * usize::from(CHANNELS)].into_boxed_slice();
            let header = Box::new(WaveHdr {
                data: samples.as_mut_ptr().cast::<c_char>(),
                buffer_length: (samples.len() * size_of::<i16>()) as u32,
                bytes_recorded: 0,
                user: 0,
                flags: 0,
                loops: 0,
                next: null_mut(),
                reserved: 0,
            });

            AudioBuffer { samples, header }
        })
        .collect()
}

fn render_buffer(buffer: &mut AudioBuffer) {
    unsafe {
        synth_audio_render_interleaved_i16(buffer.samples.as_mut_ptr(), BUFFER_FRAMES);
    }
}
