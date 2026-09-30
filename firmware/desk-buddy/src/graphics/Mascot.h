// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include "Shapes.h"
#include "Layout.h"
#include "../companion/State.h"

namespace pip {
inline void mascot(Canvas &c, const State &s, uint32_t now) {
    c.rect(PX, PY, PW, PH, STAGE);
    float t = now / 1000.f;
    bool music = s.media.connected && s.media.playing;
    bool sleep = s.mode == NAP && !music, play = s.mode == PLAY || music,
         happy = s.happy && uint32_t(now - s.happyAt) < 3500,
         wave = s.waving && uint32_t(now - s.waveAt) < 3200;
    float bob = sleep ? sinf(t * 1.5f) * 2 : (play ? -fabsf(sinf(t * 4)) * 14 : sinf(t * 2.2f) * 4);
    int cx = PX + 160 + (int)(play ? sinf(t * 4) * 7 : sinf(t * 1.1f) * 2),
        cy = PY + 166 + (int)bob;
    c.ellipse(PX + 160, PY + 277, 79 + (int)bob, 11, rgb(199, 211, 192));
    c.ellipse(PX + 160, PY + 277, 56 + (int)bob / 2, 6, rgb(183, 202, 177));
    c.ellipse(cx - 48, cy + 90, 28, 12, rgb(93, 157, 129));
    c.ellipse(cx + 48, cy + 90, 28, 12, rgb(93, 157, 129));
    float ly = cy + 37 + (play ? sinf(t * 6) * 13 : 0);
    c.line(cx - 83, cy + 11, cx - 111, ly, 22, rgb(99, 178, 143));
    c.ellipse(cx - 112, ly, 13, 16, MINT);
    float hx = cx + 112, hy = cy + 38;
    if (wave) {
        hx = cx + 118 + sinf((now - s.waveAt) * .015f) * 8;
        hy = cy - 40;
    } else if (play)
        hy = cy + 10 + sinf(t * 6 + 2) * 18;
    c.line(cx + 82, cy + 12, hx, hy, 23, rgb(99, 178, 143));
    c.ellipse(hx, hy, 14, 16, MINT);
    if (wave) {
        c.line(hx + 2, hy - 9, hx + 5, hy - 24, 7, MINT);
        c.line(hx - 6, hy - 9, hx - 9, hy - 23, 7, MINT);
    }
    c.line(cx, cy - 72, cx + 3, cy - 108, 8, rgb(82, 137, 111));
    c.ellipse(cx + 3, cy - 111, 12, 12, PEACH);
    c.ellipse(cx, cy - 114, 4, 4, rgb(255, 223, 196));
    c.round(cx - 95, cy - 80, 190, 166, 58, rgb(61, 119, 94));
    c.round(cx - 92, cy - 79, 184, 159, 55, rgb(113, 190, 148));
    c.round(cx - 88, cy - 77, 176, 145, 52, rgb(145, 215, 170));
    c.round(cx - 81, cy - 74, 162, 123, 48, rgb(171, 232, 190));
    c.ellipse(cx - 55, cy - 60, 13, 5, rgb(216, 250, 215));
    c.round(cx - 76, cy - 41, 152, 89, 31, rgb(103, 179, 141));
    c.round(cx - 73, cy - 43, 146, 85, 28, DARK);
    c.round(cx - 68, cy - 39, 136, 26, 21, rgb(35, 75, 69));
    bool blink = fmodf(t + .3f, 4.3f) > 4.13f;
    int ex = s.gazeX, ey = s.gazeY;
    if (sleep) {
        c.line(cx - 43, cy - 2, cx - 23, cy - 2, 4, MINT);
        c.line(cx + 23, cy - 2, cx + 43, cy - 2, 4, MINT);
        c.ellipse(cx + 3, cy + 24, 5, 6, MINT);
    } else if (happy || play) {
        arc(c, cx - 33, cy + 3, 11, 7, 3.35f, 6.05f, 4, MINT);
        arc(c, cx + 33, cy + 3, 11, 7, 3.35f, 6.05f, 4, MINT);
        arc(c, cx, cy + 20, 12, 6, .2f, 2.94f, 3, MINT);
    } else if (s.mode == FOCUS) {
        for (int side = -1; side <= 1; side += 2) {
            int x = cx + side * 33;
            c.round(x - 25, cy - 22, 50, 38, 12, rgb(232, 202, 131));
            c.round(x - 22, cy - 19, 44, 32, 10, DARK);
            c.ellipse(x + ex, cy - 2 + ey, 6, blink ? 2 : 8, MINT);
        }
        c.rect(cx - 9, cy - 9, 18, 3, rgb(232, 202, 131));
        c.round(cx - 6, cy + 25, 12, 3, 1, MINT);
    } else {
        for (int side = -1; side <= 1; side += 2) {
            int x = cx + side * 33 + ex;
            c.ellipse(x, cy - 4 + ey, 9, blink ? 2 : 14, rgb(197, 250, 194));
            if (!blink)
                c.ellipse(x - 2, cy - 10 + ey, 3, 3, WHITE);
        }
        arc(c, cx, cy + 19, 10, 5, .2f, 2.94f, 3, MINT);
    }
    c.ellipse(cx - 52, cy + 20, 8, 4, rgb(196, 136, 116));
    c.ellipse(cx + 52, cy + 20, 8, 4, rgb(196, 136, 116));
    c.ellipse(cx, cy + 64, 7, 7, rgb(74, 134, 105));
    c.ellipse(cx - 1, cy + 62, 3, 3, rgb(204, 246, 193));
    if (happy) {
        float age = (now - s.happyAt) / 1000.f;
        for (int i = 0; i < 4; i++) {
            float u = fmodf(age * .48f + i * .23f, 1);
            heart(c, PX + 44 + i * 77 + (int)(sinf(u * 5 + i) * 9), PY + 113 - (int)(u * 95),
                  5 + (int)((1 - u) * 5), blend(STAGE, PEACH, (int)((1 - u) * 240)));
        }
    } else if (sleep) {
        int y = PY + 46 + (int)(sinf(t) * 4);
        c.text(PX + 228, y, "z", fontHeading, MUTED);
        c.text(PX + 252, y - 24, "z", fontBody, MUTED);
    } else if (play) {
        sparkle(c, PX + 33, PY + 80 + (int)(sinf(t * 2) * 10), 10, PEACH);
        sparkle(c, PX + 283, PY + 52 + (int)(cosf(t * 2) * 9), 8, rgb(139, 175, 130));
    }
}
} // namespace pip
