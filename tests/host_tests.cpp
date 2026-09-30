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
            std::vector<uint16_t> full(1024 * 600 + 2, guard);
            Canvas canvas(full.data() + 1, 1024, 600);
            scene(canvas, state);
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
                scene(strip, state);
            }
            Canvas all(strips.data() + 1, 1024, 600);
            mascot(all, state, now);
            assert(strips == full);
        }
    }
}

int main() {
    gestures();
    rendering();
    std::puts("Gesture and full/partial rendering checks passed.");
}
