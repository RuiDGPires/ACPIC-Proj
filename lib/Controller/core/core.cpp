#include <Arduino.h>
#include <Wire.h>
#include "core.hpp"

#ifdef __TL__
#define ENTRY_NUMBER 1
#include <TrafficLights.hpp>
#endif

#define BUF_MAX 5

static int potetiometer_pin;

static char checksum(const char buffer[], int integrity_index) {
    // Checksum
    char sum = 0;
    for (int i = 0; i < integrity_index; i++)
        sum += buffer[i];

    return sum;
}

static void send_message(int to, Operation op) {
    char buffer[5];

    buffer[0] = 0;
    buffer[1] = (char) op;
    buffer[2] = (char) to;
    buffer[3] = checksum(buffer, 3);

#ifdef __TL__
    if (to == ENTRY_NUMBER) {
        tl_message(buffer);
        tl_response(buffer);
        return;
    }
#endif

    Wire.beginTransmission(to);
    Wire.write(buffer);
    Wire.endTransmission();
    Wire.requestFrom(to, op == OP_PING ? 5 : 4);

    for (int i = 0; Wire.available(); i++) {
        buffer[i] = Wire.read();
    }
}

void ct_setup(int _potetiometer_pin) {
    potetiometer_pin = _potetiometer_pin;
}

void ct_loop() {
}

