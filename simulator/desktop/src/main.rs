use minifb::{Key, Scale, Window, WindowOptions};
use pico_synth_desktop_sim::{DISPLAY_HEIGHT, DISPLAY_WIDTH, lcd_window_frame_snapshot};

unsafe extern "C" {
    fn synth_simulator_main_once() -> i32;
}

fn main() -> Result<(), minifb::Error> {
    let startup_result = unsafe { synth_simulator_main_once() };
    if startup_result != 0 {
        eprintln!("failed to initialize firmware screen buffer");
        std::process::exit(startup_result);
    }

    let mut window = Window::new(
        "PicoSynthSim - LCD 1.44\"",
        DISPLAY_WIDTH,
        DISPLAY_HEIGHT,
        WindowOptions {
            scale: Scale::X4,
            ..WindowOptions::default()
        },
    )?;

    while window.is_open() && !window.is_key_down(Key::Escape) {
        let frame = lcd_window_frame_snapshot();
        window.update_with_buffer(&frame, DISPLAY_WIDTH, DISPLAY_HEIGHT)?;
    }

    Ok(())
}
