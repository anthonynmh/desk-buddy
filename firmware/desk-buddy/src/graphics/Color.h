// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include <stdint.h>

namespace pip {
constexpr uint16_t rgb(int r, int g, int b) {
    return ((r & 248) << 8) | ((g & 252) << 3) | (b >> 3);
}
constexpr uint16_t BG = rgb(245, 243, 237), INK = rgb(30, 55, 49), MUTED = rgb(102, 119, 109),
                   STAGE = rgb(228, 236, 224), WHITE = rgb(255, 255, 250),
                   MINT = rgb(153, 221, 178), DARK = rgb(29, 64, 61), PEACH = rgb(242, 171, 137);
inline uint16_t blend(uint16_t a, uint16_t b, int k) {
    int r = (((a >> 11) & 31) * (255 - k) + ((b >> 11) & 31) * k + 127) / 255,
        g = (((a >> 5) & 63) * (255 - k) + ((b >> 5) & 63) * k + 127) / 255,
        v = ((a & 31) * (255 - k) + (b & 31) * k + 127) / 255;
    return (r << 11) | (g << 5) | v;
}
} // namespace pip
