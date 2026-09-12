use std::path::PathBuf;

fn main() {
    let root = PathBuf::from("../..");
    let firmware = root.join("firmware");

    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/synth_app.c").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/GUI_Paint.c").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/GUI_Paint.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/synth_c_api.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/hardware.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("hal/lcd_1in44.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("hal/lcd_1in44.c").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("hal/DEV_Config.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("hal/Debug.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/Fonts/font12.c").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/Fonts/fonts.h").display()
    );
    println!("cargo:rerun-if-changed=cpp/sim_main.cpp");

    cc::Build::new()
        .cpp(true)
        .std("c++17")
        .include(firmware.join("hal"))
        .include(firmware.join("synth_core"))
        .include(firmware.join("synth_core/Fonts"))
        .file(firmware.join("hal/lcd_1in44.c"))
        .file(firmware.join("synth_core/synth_app.c"))
        .file(firmware.join("synth_core/GUI_Paint.c"))
        .file(firmware.join("synth_core/Fonts/font12.c"))
        .file("cpp/sim_main.cpp")
        .compile("synth_core");
}
