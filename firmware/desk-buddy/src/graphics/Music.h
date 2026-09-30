// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include "Canvas.h"
#include "Layout.h"
#include "../companion/Media.h"

namespace pip {
inline void musicText(Canvas &c, int y, const char *value, const Font &font, uint16_t color) {
    char text[64];
    strncpy(text, value, sizeof(text) - 1); text[63] = 0;
    unsigned length = strlen(text);
    bool shortened = false;
    while (length && c.width(text, font) > 296) {
        text[--length] = 0; shortened = true;
    }
    if (shortened && length >= 3) memcpy(text + length - 3, "...", 3);
    c.text(668, y, text, font, color);
}
inline void musicPanel(Canvas &c, const Media &m, uint32_t now) {
    c.round(640, 108, 352, 424, 26, WHITE);
    c.text(668, 132, "Music with Pip", fontHeading, INK);
    c.text(668, 176, !m.connected ? "Connect the Windows helper" :
           !m.available ? "Waiting for Spotify" : m.playing ? "SPOTIFY / PLAYING" :
           "SPOTIFY / PAUSED", fontSmall, MUTED);
    musicText(c, 220, m.available ? m.title : "Your next tiny dance break", fontBody, INK);
    musicText(c, 259, m.available ? m.artist : "Play a song on your computer", fontSmall, MUTED);
    musicText(c, 300, m.available && m.album[0] ? m.album : "Album appears here", fontSmall, MUTED);
    uint32_t position = m.elapsed(now);
    int width = m.duration ? int(uint64_t(position) * (seekBar.w - 1) / m.duration) : 0;
    c.round(seekBar.x, 360, seekBar.w, 6, 3, STAGE);
    c.round(seekBar.x, 360, width, 6, 3, MINT);
    if (m.canSeek) c.ellipse(seekBar.x + width, 363, 8, 8, INK);
    char time[40];
    snprintf(time, sizeof(time), "%lu:%02lu / %lu:%02lu",
             (unsigned long)(position / 60000), (unsigned long)(position / 1000 % 60),
             (unsigned long)(m.duration / 60000), (unsigned long)(m.duration / 1000 % 60));
    c.text(668, 384, m.available ? time : "USB / waiting for connection", fontSmall, MUTED);
    c.round(playbackButton.x, playbackButton.y, playbackButton.w, playbackButton.h, 14, STAGE);
    c.centered(816, 418, !m.canToggle ? "Controls unavailable" : m.playing ? "Pause" : "Play",
               fontSmall, m.canToggle ? INK : MUTED);
}
} // namespace pip
