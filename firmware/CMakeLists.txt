set(PICO_SYNTH_TARGET pico-synth-firmware)

add_executable(${PICO_SYNTH_TARGET}
    rp2040_backend/main.cpp

    hal/lcd_1in44.c
    rp2040_backend/audio_pio.c
    rp2040_backend/DEV_Config.c
    rp2040_backend/hardware_rp2040.cpp
    rp2040_backend/lcd_1in44_rp2040.cpp
    synth_core/GUI_Paint.c
    synth_core/gui.cpp
    synth_core/synth_app.cpp
    synth_core/synthesizer.cpp
    synth_core/voice.cpp
    synth_core/Fonts/font8.c
    synth_core/Fonts/font12.c
    synth_core/Fonts/font16.c
    synth_core/Fonts/font20.c
    synth_core/Fonts/font24.c
    synth_core/Fonts/font12CN.c
    synth_core/Fonts/font24CN.c
)

pico_set_program_name(${PICO_SYNTH_TARGET} "pico-synth-firmware")
pico_set_program_version(${PICO_SYNTH_TARGET} "0.1")

pico_enable_stdio_uart(${PICO_SYNTH_TARGET} 0)
pico_enable_stdio_usb(${PICO_SYNTH_TARGET} 1)

target_include_directories(${PICO_SYNTH_TARGET} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hal
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_backend
    ${CMAKE_CURRENT_LIST_DIR}/synth_core
)

target_link_libraries(${PICO_SYNTH_TARGET} PRIVATE
    pico_stdlib
    pico_util
    pico_multicore

    hardware_gpio
    hardware_pio
    hardware_spi
    hardware_pwm
    hardware_adc
)

pico_generate_pio_header(
    ${PICO_SYNTH_TARGET}
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_backend/audio_pio.pio
)

pico_add_extra_outputs(${PICO_SYNTH_TARGET})
