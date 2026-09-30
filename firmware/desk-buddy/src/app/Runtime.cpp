#include <Arduino.h>
#include "esp_heap_caps.h"
#include "esp_lcd_panel_ops.h"
#include "Runtime.h"
#include "../hardware/rgb_lcd_port.h"
#include "../hardware/gt911.h"
#include "../companion/Interaction.h"
#include "../graphics/Scene.h"
#include "../graphics/Mascot.h"

namespace desk_buddy {
static esp_lcd_panel_handle_t panel = nullptr;
static uint16_t *actor = nullptr, *strip = nullptr;
static pip::Interaction ui;
static uint32_t lastTouch = 0, lastFrame = 0, lastReport = 0, frames = 0;
static uint64_t drawUs = 0, submitUs = 0;
static constexpr int STRIP_ROWS = 24;
static void halt(const char *message) {
    Serial.println(message);
    for (;;)
        delay(1000);
}
static void drawRegion(int x, int y, int w, int h) {
    for (int row = 0; row < h; row += STRIP_ROWS) {
        int rows = pip::imin(STRIP_ROWS, h - row);
        pip::Canvas c(strip, w, rows, x, y + row);
        pip::scene(c, ui.state);
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, x, y + row, x + w, y + row + rows, strip));
        yield();
    }
}
static void controls() {
    drawRegion(56, 124, 540, 28);
    drawRegion(56, 463, 540, 35);
    for (int i = 0; i < 4; i++) {
        const pip::Box &b = pip::buttons[i];
        drawRegion(b.x, b.y, b.w, b.h);
    }
    ui.dirty = false;
}
static void frame(uint32_t now) {
    uint32_t start = micros();
    pip::Canvas c(actor, pip::PW, pip::PH, pip::PX, pip::PY);
    pip::mascot(c, ui.state, now);
    uint32_t drawn = micros();
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, pip::PX, pip::PY, pip::PX + pip::PW,
                                              pip::PY + pip::PH, actor));
    uint32_t end = micros();
    drawUs += (uint32_t)(drawn - start);
    submitUs += (uint32_t)(end - drawn);
    frames++;
}
void begin() {
    Serial.begin(115200);
    delay(400);
    Serial.println("\ndesk-buddy / Waveshare ESP32-S3-Touch-LCD-7B");
    if (!psramFound())
        halt("PSRAM unavailable. Enable Tools > PSRAM > OPI PSRAM, then upload again.");
    Serial.printf("PSRAM: %u bytes\n", (unsigned)ESP.getPsramSize());
    // This function initializes I2C and the IO expander too; do not initialize them twice.
    touch_gt911_init();
    wavesahre_rgb_lcd_bl_off();
    panel = waveshare_esp32_s3_rgb_lcd_init();
    actor =
        (uint16_t *)heap_caps_malloc(pip::PW * pip::PH * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    strip =
        (uint16_t *)heap_caps_malloc(1024 * STRIP_ROWS * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!strip)
        strip = (uint16_t *)heap_caps_malloc(1024 * STRIP_ROWS * 2,
                                             MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!actor || !strip)
        halt("Drawing buffer allocation failed. Check 8 MB OPI PSRAM is enabled.");
    ui.state.waving = true;
    ui.state.waveAt = millis();
    drawRegion(0, 0, 1024, 600);
    ui.dirty = false;
    frame(millis());
    wavesahre_rgb_lcd_bl_on();
    lastReport = lastFrame = millis();
    frames = 0;
    drawUs = submitUs = 0;
    Serial.println("Pip is ready. Tap, brush, or choose a mood.");
}
void update() {
    uint32_t now = millis();
    if ((uint32_t)(now - lastTouch) >= 16) {
        lastTouch = now;
        touch_gt911_point_t p = touch_gt911_read_point(1);
        ui.sample(p.cnt > 0, p.x[0], p.y[0], now);
    }
    if (ui.dirty)
        controls();
    now = millis();
    if ((uint32_t)(now - lastFrame) >= 40) {
        lastFrame = now;
        ui.animate();
        frame(now);
    }
    now = millis();
    if ((uint32_t)(now - lastReport) >= 5000) {
        if (frames)
            Serial.printf(
                "Pip: %.1f updates/s | draw %.1f ms | submit %.1f ms | mood %s | pets %u\n",
                frames * 1000.f / (now - lastReport), drawUs / (frames * 1000.f),
                submitUs / (frames * 1000.f), pip::labels[ui.state.mode], ui.state.pets);
        lastReport = now;
        frames = 0;
        drawUs = submitUs = 0;
    }
    delay(1);
}

} // namespace desk_buddy
