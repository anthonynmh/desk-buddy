// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include "Canvas.h"

namespace pip {
inline void sparkle(Canvas &c, int x, int y, int r, uint16_t col) {
    c.triangle(x, y - r, x - r / 3, y, x + r / 3, y, col);
    c.triangle(x, y + r, x - r / 3, y, x + r / 3, y, col);
    c.triangle(x - r, y, x, y - r / 3, x, y + r / 3, col);
    c.triangle(x + r, y, x, y - r / 3, x, y + r / 3, col);
}
inline void heart(Canvas &c, int x, int y, int r, uint16_t col) {
    c.ellipse(x - r / 2, y, r * .58f, r * .58f, col);
    c.ellipse(x + r / 2, y, r * .58f, r * .58f, col);
    c.triangle(x - r, y, x + r, y, x, y + r * 1.4f, col);
}
inline void icon(Canvas &c, int type, int x, int y, uint16_t col, uint16_t bg) {
    if (type == 0) {
        c.round(x - 9, y - 3, 18, 19, 6, col);
        for (int i = 0; i < 4; i++)
            c.round(x - 10 + i * 5, y - 17 + (i == 0 ? 4 : 0), 4, 20, 2, col);
        c.line(x - 8, y + 10, x - 18, y - 1, 5, col);
    } else if (type == 1) {
        c.round(x - 15, y - 12, 30, 24, 5, col);
        c.rect(x - 11, y - 8, 22, 16, bg);
        c.rect(x - 2, y - 7, 3, 14, col);
    } else if (type == 2)
        sparkle(c, x, y, 19, col);
    else {
        c.ellipse(x, y, 17, 17, col);
        c.ellipse(x + 8, y - 6, 14, 14, bg);
    }
}
inline void arc(Canvas &c, float x, float y, float rx, float ry, float start, float end, int thick,
                uint16_t col) {
    for (int i = 0; i < 16; i++) {
        float a = start + (end - start) * i / 16, b = start + (end - start) * (i + 1) / 16;
        c.line(x + cosf(a) * rx, y + sinf(a) * ry, x + cosf(b) * rx, y + sinf(b) * ry, thick, col);
    }
}
} // namespace pip
