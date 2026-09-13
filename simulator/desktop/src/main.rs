mod audio;

use minifb::{Key, KeyRepeat, MouseButton, Scale, Window, WindowOptions};
use pico_synth_desktop_sim::{
    DISPLAY_HEIGHT, DISPLAY_WIDTH, HW_KEY_BUTTON_1, HW_KEY_BUTTON_2, HW_KEY_BUTTON_3,
    HW_KEY_BUTTON_4, HW_KEY_PRESSED, HW_KEY_RELEASED, HW_KEY_ROTARY_SWITCH, HW_ROTARY_CCW,
    HW_ROTARY_CW, lcd_window_frame_snapshot, push_key_event, push_rotation_event,
};

unsafe extern "C" {
    fn synth_simulator_main_once() -> i32;
    fn synth_simulator_loop_once();
}

fn main() -> Result<(), minifb::Error> {
    let startup_result = unsafe { synth_simulator_main_once() };
    if startup_result != 0 {
        eprintln!("failed to initialize firmware screen buffer");
        std::process::exit(startup_result);
    }
    audio::start_audio_thread();

    let mut window = Window::new(
        "PicoSynthSim - LCD 1.44\"",
        DISPLAY_WIDTH,
        DISPLAY_HEIGHT,
        WindowOptions {
            scale: Scale::X4,
            ..WindowOptions::default()
        },
    )?;
    let mut input_state = InputState::default();

    while window.is_open() && !window.is_key_down(Key::Escape) {
        poll_window_input(&window, &mut input_state);
        unsafe {
            synth_simulator_loop_once();
        }

        let frame = lcd_window_frame_snapshot();
        window.update_with_buffer(&frame, DISPLAY_WIDTH, DISPLAY_HEIGHT)?;
    }

    Ok(())
}

#[derive(Default)]
struct InputState {
    middle_mouse_down: bool,
}

fn poll_window_input(window: &Window, state: &mut InputState) {
    for key in window.get_keys_pressed(KeyRepeat::No) {
        if let Some(button) = map_button_key(key) {
            push_key_event(button, HW_KEY_PRESSED);
        } else if let Some(direction) = map_rotation_key(key) {
            push_rotation_event(direction);
        }
    }

    for key in window.get_keys_released() {
        if let Some(button) = map_button_key(key) {
            push_key_event(button, HW_KEY_RELEASED);
        }
    }

    let middle_mouse_down = window.get_mouse_down(MouseButton::Middle);
    if middle_mouse_down != state.middle_mouse_down {
        let key_state = if middle_mouse_down {
            HW_KEY_PRESSED
        } else {
            HW_KEY_RELEASED
        };
        push_key_event(HW_KEY_ROTARY_SWITCH, key_state);
        state.middle_mouse_down = middle_mouse_down;
    }

    if let Some((_x, y)) = window.get_scroll_wheel() {
        if y > 0.0 {
            push_rotation_event(HW_ROTARY_CW);
        } else if y < 0.0 {
            push_rotation_event(HW_ROTARY_CCW);
        }
    }
}

fn map_button_key(key: Key) -> Option<std::ffi::c_int> {
    match key {
        Key::Key1 | Key::NumPad1 => Some(HW_KEY_BUTTON_1),
        Key::Key2 | Key::NumPad2 => Some(HW_KEY_BUTTON_2),
        Key::Key3 | Key::NumPad3 => Some(HW_KEY_BUTTON_3),
        Key::Key4 | Key::NumPad4 => Some(HW_KEY_BUTTON_4),
        Key::Enter => Some(HW_KEY_ROTARY_SWITCH),
        _ => None,
    }
}

fn map_rotation_key(key: Key) -> Option<std::ffi::c_int> {
    match key {
        Key::Left => Some(HW_ROTARY_CCW),
        Key::Right => Some(HW_ROTARY_CW),
        _ => None,
    }
}
