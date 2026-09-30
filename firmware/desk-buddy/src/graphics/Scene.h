// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include "Shapes.h"
#include "Layout.h"
#include "../companion/State.h"

namespace pip {
inline void scene(Canvas &c, const State &s) {
    c.rect(0, 0, 1024, 600, BG);
    c.ellipse(49, 46, 13, 13, INK);
    c.ellipse(45, 44, 2, 3, MINT);
    c.ellipse(54, 44, 2, 3, MINT);
    c.text(72, 17, "Pip", fontTitle, INK);
    c.text(173, 32, "YOUR DESK COMPANION", fontSmall, MUTED);
    c.round(816, 30, 176, 34, 17, STAGE);
    c.ellipse(835, 47, 4, 4, rgb(73, 146, 107));
    c.text(849, 35, "HERE WITH YOU", fontSmall, INK);
    c.round(32, 108, 584, 424, 26, STAGE);
    c.text(58, 128, modes[s.mode], fontSmall, MUTED);
    sparkle(c, 116, 227, 9, rgb(180, 197, 176));
    sparkle(c, 533, 364, 12, rgb(180, 197, 176));
    c.ellipse(528, 198, 4, 4, rgb(183, 199, 179));
    c.ellipse(104, 397, 3, 3, rgb(183, 199, 179));
    c.centered(324, 466, s.message, fontBody, INK);
    c.centered(324, 504, "Tap to greet  /  brush to make me smile", fontSmall, MUTED);
    c.round(640, 108, 352, 424, 26, WHITE);
    c.text(666, 132, "A little company.", fontHeading, INK);
    c.text(666, 176, "Pick our next tiny moment.", fontSmall, MUTED);
    for (int i = 0; i < 4; i++) {
        const Box &b = buttons[i];
        bool active = s.mode == i;
        uint16_t fill = active ? rgb(220, 235, 207) : rgb(244, 244, 237);
        c.round(b.x, b.y, b.w, b.h, 18, active ? rgb(126, 158, 111) : rgb(216, 222, 211));
        c.round(b.x + 1, b.y + 1, b.w - 2, b.h - 2, 17, fill);
        icon(c, i, b.x + b.w / 2, b.y + 34, active ? INK : MUTED, fill);
        c.centered(b.x + b.w / 2, b.y + 73, labels[i], fontBody, INK);
    }
    c.centered(816, 490, "Small moments. Good company.", fontSmall, MUTED);
    c.text(34, 561, "A calm little corner of your workday.", fontSmall, MUTED);
    c.text(795, 561, "Made for tiny breaks.", fontSmall, MUTED);
}
} // namespace pip
