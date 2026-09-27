# Pip — your little desk companion

A standalone animated, touchable mascot for the **Waveshare ESP32-S3-Touch-LCD-7B, 1024 × 600**.
All display and touch drivers are included. No LVGL, Arduino_GFX, SD card, or extra example downloads are needed.

## Upload in Arduino IDE on Windows

1. Extract the **entire ZIP**, then open `PipCompanion/PipCompanion.ino`.
   Keep every `.h` and `.cpp` file beside the sketch. Open this project separately from your old FPS test.
2. Keep your working **Waveshare ESP32S3 XIP** board selection. Check these Tools settings:

   | Setting | Value |
   | --- | --- |
   | Board | Waveshare ESP32S3 XIP |
   | PSRAM | OPI PSRAM / 8 MB OPI |
   | Flash Size | 16 MB |
   | CPU Frequency | 240 MHz |
   | USB CDC On Boot | Disabled, so logs use UART |
   | Port | The board's current COM port; previously COM3 |

   Keep the other XIP settings that worked for your display test.
3. Connect the USB data cable to **UART1** and put the physical UART switch at **UART1**.
4. Click **Upload**. Press **RESET** once if it does not restart automatically.

This replaces the existing demo or FPS test. Pip runs locally after upload and needs only power.
It does not communicate with or control your computer yet.

## Interact with Pip

- **Tap Pip:** a friendly wave; tapping wakes it from a nap.
- **Stroke its head or body:** drag your finger to get smiling eyes and floating hearts.
- **Say hello:** gently breathing and blinking idle mood.
- **Focus:** quiet company with little glasses.
- **Play:** a bouncing dance with sparkles.
- **Nap:** slower breathing, closed eyes, and floating Zs.
- In its awake idle mood, Pip's eyes follow your finger.

`Preview.png` was generated using the actual app renderer.

## Animation performance

The screen is 1024 × 600, but normal animation repaints only a **320 × 302** region.
Labels and buttons update when interactions change them. Buffers are allocated once.
Animation uses elapsed time and targets 25 application updates per second; actual performance
on your hardware is not yet measured. This is not a claim about the LCD's physical refresh rate.

Open **Serial Monitor at 115200 baud** for a log every five seconds:

    Pip: ... updates/s | draw ... ms | submit ... ms | mood ... | pets ...

One persistent framebuffer supports partial redraws. Drawing is not synchronized to vertical
blank, so some tearing is possible.

## Troubleshooting

- **PSRAM unavailable:** set Tools > PSRAM to OPI PSRAM and upload again.
- **Upload cannot connect:** check the UART1 cable, physical switch, and COM port. If needed,
  hold BOOT, tap RESET, release BOOT, then upload. Reset afterward.
- **Blank screen or touch failure:** press RESET and capture startup output at 115200 baud.
- **Missing header:** extract the whole ZIP and keep the `.ino`, `.h`, and `.cpp` files together.
- Changing Tools settings requires another upload to affect the firmware on the board.

## Customize

Colors, layout, and character animation are in `PipGraphics.h`.
Touch behavior and phrases are in `PipInteraction.h`.
Hardware setup, partial updates, and performance logs are in `PipCompanion.ino`.
`PipFonts.h` embeds the fonts, so no font files need to be copied onto the device.

## Validation and limits

The renderer and gesture state machine passed host C++ address/undefined-behavior checks.
Checks cover the four moods, animation frames, partial/full rendering consistency, touch
release debounce, petting, and waking from a nap. Leak checking was disabled because it is
unsupported in the test environment. The animated preview uses this same renderer.

A full ESP32 firmware build could not be completed in the available execution environment.
The project has not been tested on a physical display here. Compile/upload it in your working
Arduino XIP installation, then check the screen, touch response, and performance logs.

## Driver sources and licenses

The included drivers come from Waveshare's official 7B `08_TOUCH` example:
https://github.com/waveshareteam/ESP32-S3-Touch-LCD-7B/tree/master/examples/Arduino/examples/08_TOUCH

Documentation: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7B

Modifications: one persistent RGB framebuffer; the unused legacy second-framebuffer output
returns null; GT911 read results are initialized to zero and return early on I2C errors.
The original 7B pins, 30 MHz pixel clock, timings, and touch initialization are retained.

Original app code: MIT (`LICENSE-Pip.txt`). Hardware drivers: Apache-2.0
(`LICENSE-Waveshare.txt` and source notices). Fonts: DejaVu/Bitstream (`LICENSE-Font.txt`).
