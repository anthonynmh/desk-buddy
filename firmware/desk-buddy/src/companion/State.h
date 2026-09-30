// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include <stdint.h>
#include "Media.h"
#include "Network.h"

namespace pip {
enum Mode { HELLO, FOCUS, PLAY, NAP };
struct State {
    Media media;
    NetworkUi network;
    Mode mode = HELLO;
    bool waving = false, happy = false;
    uint32_t waveAt = 0, happyAt = 0;
    float gazeX = 0, gazeY = 0;
    unsigned pets = 0;
    const char *message = "Hey, I'm Pip. Nice to meet you.";
};
} // namespace pip
