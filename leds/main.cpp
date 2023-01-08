#include <Arduino.h>
#include <Wire.h>
#include "Roundabout/main.hpp"
#include "Road/main.hpp"

void setup() {
    int entryNumber = getEntryNumber();
    Wire.begin(entryNumber);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    roundabout_setup();
    road_setup(1, 2, 3);
}

void loop() {
    roundabout_loop();
    road_loop();
}

void receiveEvent() {

}

void requestEvent() {

}
