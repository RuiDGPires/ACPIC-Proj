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
    road_setup();
}

void loop() {
    roundabout_loop();
    road_loop();
}

void receiveEvent() {

}

void requestEvent() {

}
