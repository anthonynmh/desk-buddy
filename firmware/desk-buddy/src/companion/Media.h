// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>

namespace pip {
struct Media {
    bool connected = false, available = false, playing = false;
    bool canSeek = false, canToggle = false;
    uint32_t token = 0, position = 0, duration = 0, receivedAt = 0;
    char title[64] = {}, artist[64] = {}, album[64] = {};
    uint32_t elapsed(uint32_t now) const {
        uint64_t value = position;
        if (playing) value += uint32_t(now - receivedAt);
        return value > duration ? duration : uint32_t(value);
    }
};

// Bounded, newline-delimited ASCII protocol. Reject entire malformed frames.
struct MediaLink {
    char line[320] = {};
    unsigned used = 0;
    bool overflow = false;
    static bool number(const char *s, uint32_t &out) {
        if (!*s) return false;
        uint64_t n = 0;
        for (; *s; ++s) {
            if (*s < '0' || *s > '9') return false;
            n = n * 10 + (*s - '0');
            if (n > 0xffffffffULL) return false;
        }
        out = uint32_t(n);
        return true;
    }
    bool parse(Media &media, uint32_t now) {
        char *fields[11] = {line};
        unsigned count = 1;
        for (char *p = line; *p; ++p) {
            if (*p == '\t') {
                if (count == 11) return false;
                *p = 0;
                fields[count++] = p + 1;
            } else if (*p < 32 || *p > 126) return false;
        }
        if (count != 11 || strcmp(fields[0], "PIP1")) return false;
        uint32_t n[7];
        for (int i = 0; i < 7; ++i)
            if (!number(fields[i + 1], n[i])) return false;
        for (int i = 0; i < 4; ++i) if (n[i] > 1) return false;
        for (int i = 8; i < 11; ++i) if (strlen(fields[i]) > 63) return false;
        if (n[5] > n[6] || (!n[0] && (n[1] || n[2] || n[3]))) return false;
        Media next;
        next.connected = true;
        next.available = n[0]; next.playing = n[1];
        next.canSeek = n[2] && n[6]; next.canToggle = n[3];
        next.token = n[4]; next.position = n[5]; next.duration = n[6];
        next.receivedAt = now;
        strcpy(next.title, fields[8]); strcpy(next.artist, fields[9]);
        strcpy(next.album, fields[10]);
        media = next;
        return true;
    }
    bool feed(char ch, Media &media, uint32_t now) {
        if (ch == '\r') return false;
        if (ch == '\n') {
            line[used] = 0;
            bool accepted = !overflow && parse(media, now);
            used = 0; overflow = false;
            return accepted;
        }
        if (ch != '\t' && (ch < 32 || ch > 126)) overflow = true;
        if (used < sizeof(line) - 1) line[used++] = ch;
        else overflow = true;
        return false;
    }
    bool expire(Media &media, uint32_t now) {
        if (!media.connected || uint32_t(now - media.receivedAt) <= 4000) return false;
        media = Media();
        return true;
    }
};
} // namespace pip
