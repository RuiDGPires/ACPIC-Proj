#ifndef ENTRY_NUMBER
#define ENTRY_NUMBER 1
#endif

#define PRINT_ONCE(msg) { \
    static bool once = true;\
    if (once) { \
        Serial.println(msg); \
        once = false; \
    } \
}
#include <Arduino.h>
#include <Wire.h>
#include <TrafficLights.hpp>

static void print_msg(const char buffer[]) {
    Serial.print(  String((int)buffer[0]) + "|");
    Serial.print(  String((int)buffer[1]) + "|");
    Serial.print(  String((int)buffer[2]) + "|");
    if (buffer[1] == OP_STATUS) {
        Serial.print(String((int)buffer[3]) + "|");
        Serial.println(String((int)buffer[4]));
    } else {
        Serial.println(String((int)buffer[3]));
    }
}

static bool request = false;
void receiveEvent(int) {
    char buffer[BUF_MAX];

    // Read incoming string
    int i = 0;
    for (; Wire.available(); i++)
        buffer[i] = Wire.read();

    if (i != 4)
        return;

    Serial.println("Message received:");
    print_msg(buffer);
    tl_message(buffer);
}

char buffer[10];
int size = 4;
void requestEvent() {
    //Serial.print("Responding: ");
    //print_msg(buffer);
    for (int i = 0; i < size; i++) {
        Wire.write(buffer[i]);
        buffer[i] = 0; // TODO ?
    }
}

void setup() {
    Serial.begin(9600);
    Wire.begin(ENTRY_NUMBER);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    tl_setup(N9(2));
    Serial.println("Traffic Lights Setup Complete. Entry number: " + String(ENTRY_NUMBER));

    PRINT_ONCE("ENTRY NUMBER: " + String(ENTRY_NUMBER));
}

void loop() {
    if (tl_response(buffer)) {
        if (buffer[1] == OP_STATUS)
            size = 5;
        else
            size = 4;
    }

    tl_loop();
}

