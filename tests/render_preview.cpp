// Render a sample music scene to a PPM file for visual inspection on a host.
#include <cstdio>
#include <vector>
#include "../firmware/desk-buddy/src/graphics/Scene.h"
#include "../firmware/desk-buddy/src/graphics/Mascot.h"

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) return 1;
    pip::State state;
    state.media.connected = state.media.available = state.media.playing = true;
    state.media.canSeek = state.media.canToggle = true;
    state.media.position = 74000;
    state.media.duration = 213000;
    strcpy(state.media.title, "A Little Company");
    strcpy(state.media.artist, "The Window Seat");
    strcpy(state.media.album, "Songs for a Slow Afternoon");
    std::vector<uint16_t> pixels(1024 * 600);
    pip::Canvas canvas(pixels.data(), 1024, 600);
    if (argc == 3) {
        state.network.open = true;
        state.network.addNetwork("Home Wi-Fi", -40, true);
        state.network.addNetwork("Guest", -68, false);
        state.network.addNetwork("Neighbour", -80, true);
        state.network.dropdown = strcmp(argv[2], "dropdown") == 0;
        state.network.chooseNetwork(state.network.dropdown ? -1 : 0);
    }
    pip::scene(canvas, state, 900);
    if (!state.network.open) pip::mascot(canvas, state, 900);
    FILE *file = fopen(argv[1], "wb");
    if (!file) return 1;
    fprintf(file, "P6\n1024 600\n255\n");
    for (uint16_t pixel : pixels) {
        fputc(((pixel >> 11) & 31) * 255 / 31, file);
        fputc(((pixel >> 5) & 63) * 255 / 63, file);
        fputc((pixel & 31) * 255 / 31, file);
    }
    return fclose(file) == 0 ? 0 : 1;
}
