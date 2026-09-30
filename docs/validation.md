# Validation

## Wi-Fi publisher integration on dev — 2026-10-01

- Host C++11 checks passed for gestures, media protocol, touchscreen Wi-Fi setup
  values and keyboard modes, setup rendering, and buffer guards.
- The ESP32 firmware compiled using the installed Waveshare ESP32S3 XIP 3.1.1
  package with OPI PSRAM, 16 MB flash, 240 MHz CPU, and USB CDC disabled. It
  uses 1184706 bytes (90%) of program storage and 46524 bytes (14%) of static
  global memory.
- Windows publisher and installer PowerShell syntax passed. The publisher's
  read-only check retrieved live Spotify title, artist, album, timeline, and
  enabled seek/play controls.
- Wi-Fi association, NVS persistence, local firewall access, publisher install,
  publisher HTTP serving, and physical UI behavior remain to be verified on the
  board and target LAN.

## Spotify integration on dev — 2026-09-30

- Python structure checks passed.
- C++11 host checks passed with Zig 0.13.0 (`-Wall -Wextra -pedantic`), including
  media framing and numeric bounds, disconnects, song-token changes during a seek,
  gesture debounce, and full/partial music-panel rendering with buffer guards.
  Zig emitted warnings from its bundled libunwind sources during first compilation.
- The sample screen from `tests/render_preview.cpp` was rendered and visually
  inspected for layout, text, slider and mood-button placement.
- Windows PowerShell syntax and the read-only Windows media API check passed.
  There were zero Spotify media sessions, so live metadata and playback commands
  could not be verified.
- Firmware compiled with the installed `esp32-XIP-3.1.1:esp32:esp32s3` package,
  `PSRAM=opi,FlashSize=16M,CPUFreq=240,CDCOnBoot=default`. The final build used
  547270 bytes of program storage and 21744 bytes of static global memory.
- No upload, physical display/touch verification, or sanitizer run was performed.
  See [computer setup and acceptance checks](computer.md).

## Earlier reorganization baseline

Checks completed on 2026-09-30:

- Python structure checks passed: matching Arduino sketch folder/file,
  local includes, Markdown links, required modules, and project rename.
- Portable host tests compiled with Zig 0.16.0 in C++11 mode on Windows.
- Host executable passed gesture checks for moods, taps, release debounce,
  petting once per gesture, waking from Nap, and gaze smoothing.
- Full-screen and partial/strip rendering matched in all four moods at
  0, 900, 3200, 4300, and 10000 ms; buffer guards remained intact.
- Source-token comparisons against the original commit verified the extracted
  graphics, interaction, and runtime algorithms (allowing entry-point and
  startup-log renaming). Driver, font, and license contents were preserved,
  with Git checkout line endings normalized; preview bytes were unchanged.

The compiler emitted warnings from its bundled C++ standard library. A full
ESP32 firmware build, sanitizers, upload, and physical display/touch tests
were not performed. No Arduino board toolchain was available in this session.

See [test commands](../tests/README.md) and [board setup](setup.md).
