#define ENTRY_NUMBER 1
#include <Arduino.h>
#include <Wire.h>
#include <TrafficLights.hpp>

static void print_msg(const char buffer[]) {
    Serial.print(  String((int)buffer[0]) + "|");
    Serial.print(  String((int)buffer[1]) + "|");
    Serial.print(  String((int)buffer[2]) + "|");
    Serial.println(String((int)buffer[3]));
}

static bool request = false;
void receiveEvent(int) {
    Serial.println("Message received:");
    char buffer[BUF_MAX];

    // Read incoming string
    for (int i = 0; Wire.available(); i++)
        buffer[i] = Wire.read();

    print_msg(buffer);
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

