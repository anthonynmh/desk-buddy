# Pip â€” your little desk companion

A standalone animated, touchable mascot for the **Waveshare ESP32-S3-Touch-LCD-7B, 1024 Ã— 600**.
All display and touch drivers are included. No LVGL, Arduino_GFX, SD card, or extra example downloads are needed.

## Upload in Arduino IDE on Windows

1. Extract the **entire ZIP**, then open `firmware/desk-buddy/desk-buddy.ino`.
   Keep the complete `src/` tree beside the sketch. Arduino compiles it recursively. Open this project separately from your old FPS test.
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
For optional Spotify playback and touch controls, use the touchscreen Wi-Fi
settings and install the [Windows publisher](computer.md) after upload.

## Interact with Pip

- **Tap Pip:** a friendly wave; tapping wakes it from a nap.
- **Stroke its head or body:** drag your finger to get smiling eyes and floating hearts.
- **Say hello:** gently breathing and blinking idle mood.
- **Focus:** quiet company with little glasses.
- **Play:** a bouncing dance with sparkles.
- **Nap:** slower breathing, closed eyes, and floating Zs.
- In its awake idle mood, Pip's eyes follow your finger.

[`Preview.png`](assets/Preview.png) was generated using the actual app renderer.

## Animation performance

The screen is 1024 Ã— 600, but normal animation repaints only a **320 Ã— 302** region.
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
- **Missing header:** extract the whole ZIP and keep the sketch and its complete `src/` tree together.
- Changing Tools settings requires another upload to affect the firmware on the board.

## Customize

See the [project map](../README.md) and [architecture](architecture.md).
Colors, layout, and animation are in `firmware/desk-buddy/src/graphics/`.
Touch behavior and phrases are in `src/companion/Interaction.h`.
Hardware setup, partial updates, and logs are in `src/app/Runtime.cpp`.
`src/assets/Fonts.h` embeds the fonts; no external font files are needed.
The `src/` paths are relative to `firmware/desk-buddy/`.

## Validation and limits

The repository includes [host checks](../tests/README.md) for gesture behavior,
buffer guards, and full/partial rendering consistency across all four moods.
The original project reported address/undefined-behavior checks; sanitizer
results must be verified again with a supporting toolchain after changes.

The music integration compiles with the locally installed Waveshare ESP32S3 XIP
3.1.1 package, OPI PSRAM, 16 MB flash, 240 MHz CPU, and USB CDC disabled.
The project has not been tested on a physical display here. Upload it in your working
Arduino XIP installation, then check the screen, touch response, and performance logs.

## Driver sources and licenses

The included drivers come from Waveshare's official 7B `08_TOUCH` example:
https://github.com/waveshareteam/ESP32-S3-Touch-LCD-7B/tree/master/examples/Arduino/examples/08_TOUCH

Documentation: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7B

Modifications: one persistent RGB framebuffer; the unused legacy second-framebuffer output
returns null; GT911 read results are initialized to zero and return early on I2C errors.
The original 7B pins, 30 MHz pixel clock, timings, and touch initialization are retained.

Original app code: MIT ([LICENSE-Pip.txt](../licenses/LICENSE-Pip.txt)). Hardware drivers: Apache-2.0
([LICENSE-Waveshare.txt](../licenses/LICENSE-Waveshare.txt) and source notices). Fonts: DejaVu/Bitstream ([LICENSE-Font.txt](../licenses/LICENSE-Font.txt)).
