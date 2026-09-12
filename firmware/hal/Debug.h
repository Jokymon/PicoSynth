#ifndef PICO_SYNTH_FIRMWARE_DEBUG_H
#define PICO_SYNTH_FIRMWARE_DEBUG_H

#ifdef PICO_SYNTH_ENABLE_DEBUG_LOG
#include <stdio.h>
#define Debug(...) printf(__VA_ARGS__)
#else
#define Debug(...)
#endif

#endif
