#pragma once

namespace desk_buddy {
// Initialize hardware and draw the first frame.
void begin();
// Poll input, redraw changed regions, and report performance.
void update();
} // namespace desk_buddy
