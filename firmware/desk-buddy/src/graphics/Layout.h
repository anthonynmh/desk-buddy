// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once

namespace pip {
constexpr int PX = 166, PY = 158, PW = 320, PH = 302;
struct Box {
    int x, y, w, h;
    bool contains(int a, int b) const { return a >= x && a < x + w && b >= y && b < y + h; }
};
static const Box buttons[] = {
    {654, 478, 78, 40}, {738, 478, 78, 40}, {822, 478, 78, 40}, {906, 478, 78, 40}};
static const Box seekBar = {668, 344, 296, 40};
static const Box playbackButton = {712, 408, 208, 44};
static const char *labels[] = {"Say hello", "Focus", "Play", "Nap"};
static const char *modes[] = {"HAPPY TO BE HERE", "IN QUIET COMPANY", "A LITTLE HAPPY DANCE",
                              "RESTING MY PIXELS"};
} // namespace pip
