#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>
#include "../firmware/desk-buddy/src/companion/Interaction.h"
#include "../firmware/desk-buddy/src/graphics/Scene.h"
#include "../firmware/desk-buddy/src/graphics/Mascot.h"

static void gestures() {
    using namespace pip;
    Interaction ui;
    for (int i = 0; i < 4; ++i) {
        ui.sample(true, buttons[i].x + 10, buttons[i].y + 10, 1000 + i * 1000);
        assert(ui.state.mode == static_cast<Mode>(i));
        assert(ui.dirty);
        ui.sample(false, 0, 0, 1200 + i * 1000);
        assert(!ui.down);
    }
    assert(ui.state.mode == NAP);
    ui.sample(true, 300, 300, 5000);
    ui.sample(false, 0, 0, 5060);
    assert(ui.down); // A brief missing sample is not a release.
    ui.sample(false, 0, 0, 5111);
    assert(!ui.down && ui.state.mode == HELLO && ui.state.waving);
    assert(ui.state.waveAt == 5111);

    ui.select(NAP, 6000);
    ui.sample(true, 300, 300, 6000);
    ui.sample(true, 340, 300, 6016);
    assert(ui.state.mode == HELLO && ui.state.happy && ui.state.pets == 1);
    ui.sample(true, 380, 300, 6032);
    assert(ui.state.pets == 1); // Count once per gesture.
    ui.animate();
    assert(ui.state.gazeX > 0 && ui.state.gazeX < 9);
    ui.sample(false, 0, 0, 6200);
    ui.sample(true, 300, 300, 6300);
    ui.sample(true, 340, 300, 6316);
    assert(ui.state.pets == 2);

    Interaction outside;
    outside.sample(true, 10, 10, 1000);
    outside.sample(false, 0, 0, 1111);
    assert(!outside.state.waving && outside.state.pets == 0);
    outside.sample(true, -1, 0, 1200);
    assert(!outside.down);
}

static void rendering() {
    using namespace pip;
    const uint16_t guard = 0xBEEF;
    for (int mode = 0; mode < 4; ++mode) {
        for (uint32_t now : {0u, 900u, 3200u, 4300u, 10000u}) {
            State state;
            state.mode = static_cast<Mode>(mode);
            state.waving = true;
            state.happy = (mode == 0);
            state.media.connected = mode != 0;
            state.media.available = mode >= 2;
            state.media.playing = mode == 3;
            state.media.canSeek = mode >= 2;
            state.media.canToggle = mode >= 2;
            state.media.duration = 240000;
            state.media.position = 125000;
            strcpy(state.media.title, "A very long title that should fit safely inside the music panel");
            strcpy(state.media.artist, "Artist");
            strcpy(state.media.album, "Album");
            std::vector<uint16_t> full(1024 * 600 + 2, guard);
            Canvas canvas(full.data() + 1, 1024, 600);
            scene(canvas, state, now);
            mascot(canvas, state, now);
            assert(full.front() == guard && full.back() == guard);

            std::vector<uint16_t> actor(PW * PH + 2, guard);
            Canvas partial(actor.data() + 1, PW, PH, PX, PY);
            mascot(partial, state, now);
            assert(actor.front() == guard && actor.back() == guard);
            for (int row = 0; row < PH; ++row)
                for (int col = 0; col < PW; ++col)
                    assert(actor[1 + row * PW + col] == full[1 + (PY + row) * 1024 + PX + col]);

            std::vector<uint16_t> strips(1024 * 600 + 2, guard);
            for (int y = 0; y < 600; y += 24) {
                Canvas strip(strips.data() + 1 + y * 1024, 1024, std::min(24, 600 - y), 0, y);
                scene(strip, state, now);
            }
            Canvas all(strips.data() + 1, 1024, 600);
            mascot(all, state, now);
            assert(strips == full);

            // Runtime updates just this panel, without touching mood buttons.
            const int panelX = 640, panelY = 108, panelW = 352, panelH = 356;
            std::vector<uint16_t> panel(panelW * panelH + 2, guard);
            Canvas musicCanvas(panel.data() + 1, panelW, panelH, panelX, panelY);
            scene(musicCanvas, state, now);
            assert(panel.front() == guard && panel.back() == guard);
            for (int row = 0; row < panelH; ++row)
                for (int col = 0; col < panelW; ++col)
                    assert(panel[1 + row * panelW + col] ==
                           full[1 + (panelY + row) * 1024 + panelX + col]);
        }
    }
}

static bool packet(pip::MediaLink &link, pip::Media &media, const char *value, uint32_t now) {
    bool accepted = false;
    for (; *value; ++value) accepted = link.feed(*value, media, now) || accepted;
    return accepted;
}
static void music() {
    using namespace pip;
    MediaLink link;
    Interaction ui;
    assert(packet(link, ui.state.media, "PIP1\t1\t1\t1\t1\t7\t1000\t200000\tSong\tArtist\tAlbum\n", 100));
    assert(ui.state.media.playing && ui.state.media.elapsed(1100) == 2000);
    assert(ui.state.media.elapsed(999999) == 200000);
    assert(!packet(link, ui.state.media, "PIP1\t1\t1\t1\t1\t7\t200001\t200000\tBad\t\t\n", 200));
    assert(!packet(link, ui.state.media, "PIP1\t1\t1\t1\t1\t4294967296\t0\t0\tBad\t\t\n", 200));
    assert(strcmp(ui.state.media.title, "Song") == 0);
    for (int i = 0; i < 400; ++i) link.feed('a', ui.state.media, 200);
    assert(!link.feed('\n', ui.state.media, 200));
    ui.select(NAP, 200);
    ui.sample(true, seekBar.x, seekBar.y + 10, 300);
    ui.sample(true, seekBar.x + seekBar.w + 50, seekBar.y, 316);
    assert(ui.seekPosition == 200000 && ui.seeking);
    ui.sample(false, 0, 0, 380);
    assert(ui.down && !ui.command[0]);
    ui.sample(false, 0, 0, 427);
    assert(strcmp(ui.command, "PIP_SEEK\t7\t200000") == 0);
    assert(ui.state.mode == NAP && ui.state.pets == 0);
    ui.command[0] = 0;
    ui.sample(true, seekBar.x + 10, seekBar.y, 500);
    ui.state.media.token = 8; // Never seek into a different song on release.
    ui.sample(false, 0, 0, 611);
    assert(!ui.command[0]);
    ui.sample(true, playbackButton.x + 10, playbackButton.y + 10, 700);
    assert(strcmp(ui.command, "PIP_TOGGLE\t8") == 0);
    ui.command[0] = 0;
    ui.sample(false, 0, 0, 811);
    assert(link.expire(ui.state.media, 4101));
    assert(!ui.state.media.connected && !ui.state.media.playing && ui.state.mode == NAP);
    ui.sample(true, seekBar.x + 10, seekBar.y, 4200);
    ui.sample(false, 0, 0, 4311);
    assert(!ui.command[0]);
    assert(packet(link, ui.state.media, "PIP1\t0\t0\t0\t0\t9\t0\t0\t\t\t\n", 4400));
    assert(ui.state.media.connected && !ui.state.media.available);
    ui.state.media.receivedAt = 0xffffff00u;
    assert(link.expire(ui.state.media, 4000)); // millis rollover.
}

static void network_setup() {
    using namespace pip;
    Interaction ui;
    ui.sample(true, networkButton.x + 1, networkButton.y + 1, 100);
    assert(ui.state.network.open && ui.buttonGesture);
    ui.sample(false, 0, 0, 220);
    ui.sample(true, networkFields[0].x + 1, networkFields[0].y + 1, 300);
    ui.sample(false, 0, 0, 420);
    ui.sample(true, networkKeyBox(0).x + 1, networkKeyBox(0).y + 1, 500);
    assert(strcmp(ui.state.network.ssid, "Q") == 0);
    ui.sample(false, 0, 0, 620);
    ui.sample(true, networkKeyboard.x + 1, networkKeyboard.y + 1, 700);
    assert(ui.state.network.symbols);
    ui.sample(false, 0, 0, 820);
    ui.sample(true, networkKeyBox(0).x + 1, networkKeyBox(0).y + 1, 900);
    assert(strcmp(ui.state.network.ssid, "Q1") == 0);
    ui.state.network.nextKeyboard();
    assert(ui.state.network.lowerCase && !ui.state.network.symbols);
    ui.state.network.append('a');
    assert(strcmp(ui.state.network.ssid, "Q1a") == 0);
    ui.state.network.nextKeyboard();
    assert(ui.state.network.symbols && ui.state.network.moreSymbols &&
           ui.state.network.keyLabel(0) == '+');
    assert(NetworkUi::portNumber("8791") == 8791 && !NetworkUi::portNumber("0") &&
           !NetworkUi::portNumber("70000") && !NetworkUi::portNumber("8x"));
    strcpy(ui.state.network.host, "192.168.1.9");
    strcpy(ui.state.network.key, "ABCDEFGHIJKLMNOP");
    ui.state.network.select(PUBLISHER_PORT);
    ui.sample(false, 0, 0, 1011);
    ui.sample(true, networkSave.x + 1, networkSave.y + 1, 1100);
    assert(ui.state.network.saveRequested && ui.state.network.status == NETWORK_CONNECTING);
    ui.sample(false, 0, 0, 1211);
    ui.sample(true, networkForget.x + 1, networkForget.y + 1, 1300);
    assert(ui.state.network.forgetRequested);

    State state;
    state.network.open = true;
    strcpy(state.network.ssid, "HomeWiFi");
    std::vector<uint16_t> full(1024 * 600 + 2, 0xBEEF);
    Canvas c(full.data() + 1, 1024, 600);
    scene(c, state, 0);
    assert(full.front() == 0xBEEF && full.back() == 0xBEEF);
}

int main() {
    gestures();
    music();
    network_setup();
    rendering();
    std::puts("Gesture and full/partial rendering checks passed.");
}
