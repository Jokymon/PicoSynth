use std::path::PathBuf;

fn main() {
    let root = PathBuf::from("../..");
    let firmware = root.join("firmware");

    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/synth_demo.cpp").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/synth_c_api.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("hal/lcd_1in44.h").display()
    );

    cc::Build::new()
        .cpp(true)
        .std("c++17")
        .include(firmware.join("hal"))
        .include(firmware.join("synth_core"))
        .file(firmware.join("synth_core/synth_demo.cpp"))
        .compile("synth_core");
}
