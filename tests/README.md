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

If your compiler supports them, also run with address and undefined-behavior
sanitizers. Host tests cannot establish ESP32 firmware build or hardware health.
Open `firmware/desk-buddy/desk-buddy.ino` in Arduino IDE and use the board
settings in `docs/setup.md` for firmware verification.
