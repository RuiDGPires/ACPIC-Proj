#include <Arduino.h>
#include <Wire.h>
#include "core.hpp"

#define BUF_MAX 5

static int potetiometer_pin;

void ct_setup(int _potetiometer_pin) {
    potetiometer_pin = _potetiometer_pin;
}

char checksum(const char buffer[], int integrity_index) {
    // Checksum
    char sum = 0;
    for (int i = 0; i < integrity_index; i++)
        sum += buffer[i];

    return sum;
}


void ct_loop() {
}

