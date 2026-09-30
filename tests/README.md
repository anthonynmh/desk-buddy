# Validation

Run these commands from the repository root.

## Structure (Python 3)

```sh
python tests/check_structure.py
```

Checks sketch naming, local header paths, required module files, Markdown
links, and legacy project-name references. This is not a firmware build.

## Portable behavior and rendering (C++11 or newer)

```sh
g++ -std=c++11 -Wall -Wextra -pedantic tests/host_tests.cpp -o host-tests
./host-tests
```

On Windows, run `./host-tests.exe`. Clang can replace g++. Zig can also build it:

```sh
zig c++ -std=c++11 tests/host_tests.cpp -o host-tests.exe
```

Tests cover button selection, tapping, release debounce, petting once per
gesture, waking from Nap, and gaze smoothing. Rendering checks compare full
and partial scene/mascot output across all moods and several timestamps, with
guard pixels around the buffers. Assertions must remain enabled.

Media checks cover malformed/oversized packets, numeric overflow, reconnect and
timeout including clock rollover, seek capture and release debounce, clamping,
track changes during a drag, and play/pause commands. Wi-Fi settings checks cover
field selection, upper/lower/symbol keyboard modes, validation, save and forget.
They also check network deduplication and signal order, bounded scan lists,
selection/password clearing, open networks, held Back presses, and full/strip
rendering of setup, scan, dropdown, and computer pairing pages.
Rendering includes connected, paused and playing states, long metadata, the Wi-Fi
setup view, and partial panel painting.

Run `powershell.exe -NoProfile -File desktop/Start-DeskBuddyPublisher.ps1 -Check`
on Windows to check WinRT access without opening a network listener or changing playback. Follow
[physical acceptance checks](../docs/computer.md) for end-to-end verification.

To render a sample music screen for visual inspection, compile
`tests/render_preview.cpp` with the same compiler and save its output in an ignored
directory, for example:

```sh
g++ -std=c++11 tests/render_preview.cpp -o build/render-preview
./build/render-preview build/music-preview.ppm
```

Create `build` first. PPM output uses the actual firmware renderer and sample metadata.

If your compiler supports them, also run with address and undefined-behavior
sanitizers. Host tests cannot establish ESP32 firmware build or hardware health.
Open `firmware/desk-buddy/desk-buddy.ino` in Arduino IDE and use the board
settings in `docs/setup.md` for firmware verification.
