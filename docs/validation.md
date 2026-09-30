# Reorganization validation

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
