#include <Wire.h>
#include "core/core.cpp"

#ifndef ENTRY_NUMBER
#define ENTRY_NUMBER 1
#endif

void receiveEvent(int) {
    char buffer[20];

    // Read incoming string
    for (int i = 0; Wire.available(); i++)
        buffer[i] = Wire.read();
}

void requestEvent() {

}

void setup() {
    Wire.begin(ENTRY_NUMBER);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    tl_setup(0, 0, 0, 0, 0, 0);
}

void loop() {
    tl_loop();
}
