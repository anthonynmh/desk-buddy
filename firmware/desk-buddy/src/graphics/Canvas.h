// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include "Math.h"
#include "Color.h"
#include "../assets/Fonts.h"

namespace pip {
struct Canvas {
    uint16_t *p;
    int w, h, ox, oy;
    Canvas(uint16_t *v, int a, int b, int x = 0, int y = 0) : p(v), w(a), h(b), ox(x), oy(y) {}
    void rect(int x, int y, int rw, int rh, uint16_t color) {
        int l = imax(0, x - ox), r = imin(w, x + rw - ox), t = imax(0, y - oy),
            b = imin(h, y + rh - oy);
        for (int j = t; j < b; j++)
            for (int i = l; i < r; i++)
                p[j * w + i] = color;
    }
    void dot(int x, int y, uint16_t color, int a = 255) {
        x -= ox;
        y -= oy;
        if (x >= 0 && x < w && y >= 0 && y < h)
            p[y * w + x] = blend(p[y * w + x], color, a);
    }
    void ellipse(float x, float y, float rx, float ry, uint16_t color) {
        if (rx <= 0 || ry <= 0)
            return;
        for (int j = imax(oy, (int)floorf(y - ry)); j <= imin(oy + h - 1, (int)ceilf(y + ry));
             j++) {
            float k = (j + .5f - y) / ry;
            if (fabsf(k) >= 1)
                continue;
            float d = rx * sqrtf(1 - k * k);
            int l = ceilf(x - d), r = floorf(x + d);
            rect(l, j, r - l + 1, 1, color);
            dot(l - 1, j, color, (int)(clampf(l - x + d, 0, 1) * 180));
            dot(r + 1, j, color, (int)(clampf(x + d - r, 0, 1) * 180));
        }
    }
    void round(int x, int y, int rw, int rh, int rad, uint16_t color) {
        rad = imin(rad, imin(rw, rh) / 2);
        for (int j = imax(y, oy); j < imin(y + rh, oy + h); j++) {
            float d =
                j < y + rad ? y + rad - j - .5f : (j >= y + rh - rad ? j + .5f - y - rh + rad : 0);
            float inset = d > 0 ? rad - sqrtf(fmaxf(0, rad * rad - d * d)) : 0;
            int a = ceilf(inset);
            rect(x + a, j, rw - a * 2, 1, color);
            if (a) {
                dot(x + a - 1, j, color, (int)((a - inset) * 220));
                dot(x + rw - a, j, color, (int)((a - inset) * 220));
            }
        }
    }
    void line(float x, float y, float u, float v, int size, uint16_t color) {
        int n = imax(1, (int)(hypotf(u - x, v - y) / 2));
        for (int i = 0; i <= n; i++) {
            float t = (float)i / n;
            ellipse(x + (u - x) * t, y + (v - y) * t, size * .5f, size * .5f, color);
        }
    }
    void triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color) {
        int xs[] = {x0, x1, x2}, ys[] = {y0, y1, y2};
        for (int y = imax(oy, imin(y0, imin(y1, y2)));
             y <= imin(oy + h - 1, imax(y0, imax(y1, y2))); y++) {
            float hits[3], py = y + .5f;
            int n = 0;
            for (int i = 0; i < 3; i++) {
                int j = (i + 1) % 3;
                if ((ys[i] <= py && ys[j] > py) || (ys[j] <= py && ys[i] > py))
                    hits[n++] = xs[i] + (py - ys[i]) * (xs[j] - xs[i]) / (ys[j] - ys[i]);
            }
            if (n >= 2) {
                int l = ceilf(fminf(hits[0], hits[1])), r = floorf(fmaxf(hits[0], hits[1]));
                rect(l, y, r - l + 1, 1, color);
            }
        }
    }
    int width(const char *s, const Font &f) {
        int v = 0;
        for (; *s; s++) {
            unsigned ch = (unsigned char)*s;
            v += f.glyphs[(ch >= 32 && ch <= 126 ? ch : '?') - 32].advance;
        }
        return v;
    }
    void text(int x, int y, const char *s, const Font &f, uint16_t color) {
        for (; *s; s++) {
            unsigned ch = (unsigned char)*s;
            const Glyph &g = f.glyphs[(ch >= 32 && ch <= 126 ? ch : '?') - 32];
            int gx = x + g.dx, gy = y + g.dy;
            if (gx + g.w > ox && gx < ox + w && gy + g.h > oy && gy < oy + h)
                for (int j = 0; j < g.h; j++)
                    for (int i = 0; i < g.w; i++) {
                        int a = f.pixels[g.offset + j * g.w + i];
                        if (a)
                            dot(gx + i, gy + j, color, a);
                    }
            x += g.advance;
        }
    }
    void centered(int x, int y, const char *s, const Font &f, uint16_t color) {
        text(x - width(s, f) / 2, y, s, f, color);
    }
};
} // namespace pip
