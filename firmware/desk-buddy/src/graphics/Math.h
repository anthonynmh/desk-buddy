// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include <math.h>

namespace pip {
inline int imin(int a, int b) { return a < b ? a : b; }
inline int imax(int a, int b) { return a > b ? a : b; }
inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
} // namespace pip
