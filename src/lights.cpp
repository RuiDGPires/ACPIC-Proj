#define ENTRY_NUMBER 1
#include <Arduino.h>
#include <Wire.h>
#include <TrafficLights.hpp>

static bool request = false;
void receiveEvent(int) {
    char buffer[BUF_MAX];

    // Read incoming string
    for (int i = 0; Wire.available(); i++)
        buffer[i] = Wire.read();

    tl_message(buffer);
}

void requestEvent() {
    request = true;
}

void setup() {
    Serial.begin(9600);
    Wire.begin(ENTRY_NUMBER);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    tl_setup(N9(2));
    Serial.println("Traffic Lights Setup Complete");
}

char buffer[10];
void loop() {
    if (request) {
        if (tl_response(buffer)) {
            Wire.write(buffer);
            request = false;
        }
    }

    tl_loop();
}

