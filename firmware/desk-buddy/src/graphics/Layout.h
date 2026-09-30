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
static const Box networkButton = {816, 30, 176, 34};
static const Box networkFields[] = {{72, 126, 424, 42}, {528, 126, 424, 42},
                                    {72, 196, 424, 42}, {528, 196, 424, 42},
                                    {72, 266, 180, 42}};
static const Box networkSave = {676, 266, 156, 42}, networkForget = {848, 266, 104, 42},
                 networkClose = {912, 38, 72, 36};
inline Box networkKeyBox(int index) {
    static const int counts[] = {10, 9, 7};
    int row = index < 10 ? 0 : (index < 19 ? 1 : 2);
    int col = index - (row == 0 ? 0 : (row == 1 ? 10 : 19));
    int width = 72, start = (1024 - counts[row] * width) / 2;
    return {start + col * width, 348 + row * 52, width - 5, 44};
}
static const Box networkSpace = {308, 504, 300, 44}, networkBack = {620, 504, 116, 44},
                 networkDot = {748, 504, 72, 44}, networkDash = {832, 504, 72, 44};
static const Box networkKeyboard = {916, 504, 72, 44};
static const char *labels[] = {"Say hello", "Focus", "Play", "Nap"};
static const char *modes[] = {"HAPPY TO BE HERE", "IN QUIET COMPANY", "A LITTLE HAPPY DANCE",
                              "RESTING MY PIXELS"};
} // namespace pip
