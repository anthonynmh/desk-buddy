#pragma once
#include "State.h"
#include "../graphics/Math.h"
#include "../graphics/Layout.h"
namespace pip {
struct Interaction {
    State state;
    bool dirty = true, down = false, buttonGesture = false, petted = false;
    uint32_t lastSeen = 0, pressedAt = 0;
    int lastX = 0, lastY = 0;
    float travel = 0, targetX = 0, targetY = 0;
    bool seeking = false;
    uint32_t seekToken = 0, seekPosition = 0;
    char command[80] = {};
    void updateSeek(int x) {
        int offset = imax(0, imin(seekBar.w - 1, x - seekBar.x));
        seekPosition = uint32_t(uint64_t(offset) * state.media.duration / (seekBar.w - 1));
        dirty = true;
    }
    bool onPip(int x, int y) { return Box{190, 185, 280, 265}.contains(x, y); }
    void select(Mode m, uint32_t now) {
        state.mode = m;
        state.happy = false;
        state.waving = m == HELLO;
        state.waveAt = now;
        static const char *messages[] = {"Hey! Glad you're here.", "One thing at a time. I'm here.",
                                         "A tiny dance break? Yes, please.",
                                         "Recharging my little daydreams."};
        state.message = messages[m];
        dirty = true;
    }
    void release(uint32_t now) {
        if (!down)
            return;
        if (seeking && state.media.connected && state.media.canSeek &&
            seekToken == state.media.token)
            snprintf(command, sizeof(command), "PIP_SEEK\t%lu\t%lu",
                     (unsigned long)seekToken, (unsigned long)seekPosition);
        seeking = false;
        if (!buttonGesture && !petted && travel < 35 && now - pressedAt < 900 &&
            onPip(lastX, lastY)) {
            if (state.mode == NAP)
                select(HELLO, now);
            state.waving = true;
            state.waveAt = now;
            state.message = "Hello, you. A little wave from me.";
            dirty = true;
        }
        down = false;
        targetX = targetY = 0;
    }
    void sample(bool present, int x, int y, uint32_t now) {
        if (!present) {
            if (down && now - lastSeen > 110)
                release(now);
            return;
        }
        if (x < 0 || x >= 1024 || y < 0 || y >= 600)
            return;
        if (down && now - lastSeen > 110)
            release(now);
        if (!down) {
            down = true;
            pressedAt = now;
            travel = 0;
            buttonGesture = petted = false;
            lastX = x;
            lastY = y;
            if (seekBar.contains(x, y)) {
                buttonGesture = true;
                seeking = state.media.connected && state.media.canSeek;
                seekToken = state.media.token;
            } else if (playbackButton.contains(x, y)) {
                buttonGesture = true;
                if (state.media.connected && state.media.canToggle)
                    snprintf(command, sizeof(command), "PIP_TOGGLE\t%lu",
                             (unsigned long)state.media.token);
            }
            for (int i = 0; i < 4; i++)
                if (buttons[i].contains(x, y)) {
                    select((Mode)i, now);
                    buttonGesture = true;
                    break;
                }
        }
        lastSeen = now;
        if (seeking) {
            if (!state.media.connected || !state.media.canSeek || seekToken != state.media.token)
                seeking = false;
            else updateSeek(x);
        }
        float dx = x - lastX, dy = y - lastY;
        travel += hypotf(dx, dy);
        lastX = x;
        lastY = y;
        targetX = clampf((x - 326) * .045f, -9, 9);
        targetY = clampf((y - 320) * .032f, -5, 5);
        if (!buttonGesture && onPip(x, y) && travel >= 35) {
            if (!petted) {
                if (state.mode == NAP)
                    select(HELLO, now);
                state.pets++;
                state.message = "That makes my pixels happy.";
                dirty = true;
                petted = true;
            }
            if (dx * dx + dy * dy > 1 || !state.happy) {
                state.happy = true;
                state.happyAt = now;
            }
        }
    }
    void animate() {
        state.gazeX += (targetX - state.gazeX) * .25f;
        state.gazeY += (targetY - state.gazeY) * .25f;
    }
};
} // namespace pip
