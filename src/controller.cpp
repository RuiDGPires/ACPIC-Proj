#include <Arduino.h>
#include <Controller.hpp>
#include <Wire.h>

void setup() {
    ct_setup(3, 3);
}

void loop() {
    ct_loop();
}
