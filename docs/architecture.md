# Architecture

The app has two flows:

```text
desk-buddy.ino → app/Runtime.cpp → hardware initialization and update loop
touch coordinates → companion/Interaction → companion/State → graphics → LCD
```

## Modules

### app

`Runtime.h` exposes `desk_buddy::begin()` and `desk_buddy::update()` to the
sketch. `Runtime.cpp` owns display handles, buffers, interaction state, and
timing counters. Startup initializes touch (including I2C and the IO expander),
initializes the display, allocates buffers, paints the screen, and enables
the backlight. Do not initialize I2C twice.

The loop polls touch every 16 ms, updates the mascot at intervals of at least
40 ms, and prints performance statistics every five seconds. Animation uses
elapsed time. These are scheduling targets, not measured LCD refresh rates.

### companion

`State.h` defines the four moods and transient wave, happiness, gaze, message,
and pet-count fields. `Interaction.h` converts input samples into that state:

- A short tap on Pip waves; a tap during Nap also selects Hello.
- Movement of at least 35 pixels while touching Pip registers petting once
  per gesture, wakes Pip, and refreshes its happy expression.
- Mood buttons select immediately on press.
- Missing touch samples need more than 110 ms before release, avoiding
  accidental releases from brief sampling gaps.
- `animate()` smooths the gaze toward the finger position.

This module has no Arduino or display-driver dependency. It shares hitboxes
with `graphics/Layout.h`, but does not depend on canvas or font data.

### graphics

- `Math.h`: small numeric helpers used by drawing and gestures.
- `Color.h`: RGB565 color conversion, palette, and blending.
- `Canvas.h`: clipped pixel drawing and embedded-font text rendering.
- `Layout.h`: shared button hitboxes, labels, and mascot redraw bounds.
- `Shapes.h`: hearts, sparkles, icons, and arcs.
- `Scene.h`: background, headings, messages, and mood controls.
- `Mascot.h`: time-based character rendering from a read-only state.

Graphics stays independent of ESP32 APIs so it can be checked on a host.
Each canvas has a global origin, letting the same renderer draw either the
full screen or a small region without changing coordinates.

### assets

`Fonts.h` stores the existing embedded font tables. Treat these as generated
data; preserve their license and avoid duplicating tables across compiled
modules. Currently only the runtime includes the firmware renderer.

### hardware

The included Waveshare files implement RGB display output, GT911 touch,
generic touch support, I2C, and the IO expander. Their filenames, notices,
pins, clock, and timings are retained. Board-dependent changes belong here.

## Drawing and memory

The driver owns a persistent full-screen framebuffer. The app allocates a
320 × 302 RGB565 actor buffer in PSRAM and a 24-row drawing strip, preferring
internal RAM for the strip. Buffers are allocated once at startup.

The scene is painted fully at startup. Changed messages and buttons redraw
in strips. Every animation update repaints only the actor region. This
refactor preserves coordinates, gestures, timing, and drawing algorithms.

## Build boundary

The matching sketch directory and file are `desk-buddy/desk-buddy.ino`.
All firmware C++ lives under its `src/` directory, which Arduino compiles
recursively. Host tests include portable headers only. They cannot verify
driver compilation, memory availability on the board, upload, or touch wiring.
