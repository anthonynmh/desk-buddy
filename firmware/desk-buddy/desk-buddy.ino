// desk-buddy: Pip on the Waveshare ESP32-S3-Touch-LCD-7B.
// Upload instructions: ../../docs/setup.md
#include "src/app/Runtime.h"

void setup() { desk_buddy::begin(); }

void loop() { desk_buddy::update(); }
