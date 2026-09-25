use std::path::{Path, PathBuf};
use std::process::Command;

fn main() {
    let root = PathBuf::from("../..");
    let firmware = root.join("firmware");

    println!("cargo:rustc-link-lib=winmm");
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/synth_app.cpp").display()
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
        firmware.join("synth_core/gui.cpp").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/gui/main_menu.cpp").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/gui/widgets.cpp").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/gui.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/synthesizer.cpp").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/synthesizer.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/voice.cpp").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/voice.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        firmware.join("synth_core/midi.h").display()
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

    let sources = [
        firmware.join("hal/lcd_1in44.c"),
        firmware.join("synth_core/synth_app.cpp"),
        firmware.join("synth_core/gui.cpp"),
        firmware.join("synth_core/gui/main_menu.cpp"),
        firmware.join("synth_core/gui/widgets.cpp"),
        firmware.join("synth_core/synthesizer.cpp"),
        firmware.join("synth_core/voice.cpp"),
        firmware.join("synth_core/GUI_Paint.c"),
        firmware.join("synth_core/Fonts/font12.c"),
        PathBuf::from("cpp/sim_main.cpp"),
    ];

    let mut build = cc::Build::new();
    build
        .cpp(true)
        .std("c++17")
        .cargo_debug(true)
        .include(firmware.join("hal"))
        .include(firmware.join("synth_core"))
        .include(firmware.join("synth_core/Fonts"));

    for source in &sources {
        build.file(source);
    }

    if let Err(error) = build.try_compile("synth_core") {
        replay_compiler_diagnostics(&build, &sources);

        panic!(
            "failed to compile desktop simulator C/C++ firmware bridge: {error}\n\
             compiler diagnostics were replayed to stderr above this message."
        );
    }
}

fn replay_compiler_diagnostics(build: &cc::Build, sources: &[PathBuf]) {
    eprintln!("cc-rs failed; replaying compiler diagnostics with captured output:");

    let compiler = build.get_compiler();
    let target = std::env::var("TARGET").unwrap_or_default();
    let out_dir = PathBuf::from(std::env::var_os("OUT_DIR").unwrap());

    for (index, source) in sources.iter().enumerate() {
        let mut command = compiler.to_command();
        let object = out_dir.join(format!("diagnostic-{index}.obj"));

        add_compile_only_args(&mut command, &target, source, &object);
        eprintln!("running: {command:?}");

        match command.output() {
            Ok(output) if output.status.success() => {}
            Ok(output) => {
                eprintln!("exit status: {}", output.status);
                print_stream("stdout", &output.stdout);
                print_stream("stderr", &output.stderr);
                return;
            }
            Err(error) => {
                eprintln!("failed to replay compiler command: {error}");
                return;
            }
        }
    }
}

fn add_compile_only_args(command: &mut Command, target: &str, source: &Path, object: &Path) {
    if target.contains("msvc") {
        command.arg(format!("-Fo{}", object.display()));
        command.arg("-c");
        command.arg(source);
    } else {
        command.arg("-o");
        command.arg(object);
        command.arg("-c");
        command.arg(source);
    }
}

fn print_stream(name: &str, bytes: &[u8]) {
    if bytes.is_empty() {
        return;
    }

    eprintln!("--- compiler {name} ---");
    eprintln!("{}", String::from_utf8_lossy(bytes));
}
