#ifndef N_ENTRIES
#define N_ENTRIES 1
#endif

#include <Arduino.h>
#include <Controller.hpp>
#include <Wire.h>

void setup() {
    Serial.begin(9600);
    ct_setup(4, A0, 3);
    Serial.println("Controller Setup Complete");
}

void loop() {
    ct_loop();
}
