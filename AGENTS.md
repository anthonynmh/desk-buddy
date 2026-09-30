# Working on desk-buddy

## Orientation

Read `README.md`, `docs/architecture.md`, and `docs/setup.md` before changing
firmware. The Git root is this directory. The Arduino sketch is
`firmware/desk-buddy/desk-buddy.ino`; its folder and filename must match.

## Boundaries

- Keep the sketch as a thin entry point. Runtime orchestration goes in `src/app/`.
- Put character state and gestures in `src/companion/`.
- Keep graphics and companion logic independent of Arduino/ESP32 APIs.
- Keep rendering in `src/graphics/` and generated font data in `src/assets/`.
- Keep board-specific code in `src/hardware/`. Preserve upstream notices.
- Use `src/` for firmware subdirectories so Arduino compiles them recursively.
- Preserve included driver pins, clock, timing, and initialization order unless
  the requested task requires a hardware change. Touch init already sets up I2C.
- Preserve partial redraws and once-only buffer allocation.

## Validation

Run the host checks from the repository root as described in `tests/README.md`:

```sh
python tests/check_structure.py
g++ -std=c++11 -Wall -Wextra -pedantic tests/host_tests.cpp -o host-tests
./host-tests
```

When refactoring drawing, compare full and partial rendering and check buffer
bounds. For gesture changes, check taps, movement, release debounce, mood
buttons, and waking from Nap. Build the sketch with the installed Waveshare
ESP32S3 XIP configuration when available. Do not invent a board FQBN; inspect
the local setup first. Host results do not establish hardware correctness.
State explicitly when firmware compilation or physical verification is unavailable.

## Documentation and changes

Update the README project map and affected docs when moving files or changing
setup or responsibilities. Preserve all three licenses and font provenance.
Use readable C++ and focused changes. Keep build output and temporary tooling
out of Git. Follow the user's requested branch and commit workflow; commit in
logical stages. Verify the remote head before a requested force push, and use
an explicit `--force-with-lease` expectation. Do not overwrite user changes.
