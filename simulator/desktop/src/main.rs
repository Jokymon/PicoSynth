use minifb::{Key, Scale, Window, WindowOptions};
use pico_synth_desktop_sim::{
    DISPLAY_HEIGHT, DISPLAY_PIXELS, DISPLAY_WIDTH, lcd_framebuffer_snapshot,
};

unsafe extern "C" {
    fn synth_demo_render_frame();
}

fn main() -> Result<(), minifb::Error> {
    unsafe {
        synth_demo_render_frame();
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

    let mut frame = [0_u32; DISPLAY_PIXELS];

    while window.is_open() && !window.is_key_down(Key::Escape) {
        let pixels = lcd_framebuffer_snapshot();
        for (target, source) in frame.iter_mut().zip(pixels.iter()) {
            *target = source.to_xrgb8888();
        }

        window.update_with_buffer(&frame, DISPLAY_WIDTH, DISPLAY_HEIGHT)?;
    }

    Ok(())
}
