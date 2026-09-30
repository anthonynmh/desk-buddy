// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once

namespace pip {
constexpr int PX = 166, PY = 158, PW = 320, PH = 302;
struct Box {
    int x, y, w, h;
    bool contains(int a, int b) const { return a >= x && a < x + w && b >= y && b < y + h; }
};
static const Box buttons[] = {
    {662, 220, 148, 112}, {826, 220, 148, 112}, {662, 350, 148, 112}, {826, 350, 148, 112}};
static const char *labels[] = {"Say hello", "Focus", "Play", "Nap"};
static const char *modes[] = {"HAPPY TO BE HERE", "IN QUIET COMPANY", "A LITTLE HAPPY DANCE",
                              "RESTING MY PIXELS"};
} // namespace pip
