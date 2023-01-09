#include <Arduino.h>
#include <Wire.h>
#include "core.hpp"

#ifdef __TL__
#define ENTRY_NUMBER 1
#include <TrafficLights.hpp>
#endif

#define BUF_MAX 5

typedef enum {
    STATE_OFF,
    STATE_ENTRY,
    STATE_NORMAL
} State;

static int potetiometer_pin, button_pin;
static State state = STATE_OFF;
static unsigned int control_period = 2;
static int current = 1;

static char checksum(const char buffer[], int integrity_index) {
    // Checksum
    char sum = 0;
    for (int i = 0; i < integrity_index; i++)
        sum += buffer[i];

    return sum;
}

static void ct_send_message(int to, Operation op) {
    char buffer[BUF_MAX];

    buffer[0] = 0;
    buffer[1] = (char) op;
    buffer[2] = (char) to;
    buffer[3] = checksum(buffer, 3);

    int size = op == OP_PING ? 5 : 4;
#ifdef __TL__
    if (to == ENTRY_NUMBER) {
        tl_message(buffer);
        tl_response(buffer);
        goto parse_response;
    }
#endif

    Wire.beginTransmission(to);
    Wire.write(buffer);
    Wire.endTransmission();
    Wire.requestFrom(to, size);

    for (int i = 0; Wire.available(); i++) {
        buffer[i] = Wire.read();
    }

parse_response:;
    if (buffer[1] == OP_STATUS) {
        // TODO 
    }
}

#define WAIT_FOR(time) {\
    start = millis(); \
    stop = start + (time); \
    wait = true; \
}

void ct_setup(int _potetiometer_pin, int _button_pin) {
    potetiometer_pin = _potetiometer_pin;
    button_pin = _button_pin;
}


void ct_loop() {
    static bool wait = false;
    static uint32_t start = 0, stop = 0;

    if (wait) {
        if (millis() >= stop) {
            wait = false;
            start = stop = 0;
        }
    } else {
        switch (state) {
            case STATE_OFF:
                break;
            case STATE_ENTRY:
                ct_send_message(2, OP_RED);
                ct_send_message(3, OP_RED);
                ct_send_message(4, OP_RED);
                //--
                ct_send_message(1, OP_GREEN);
                state = STATE_NORMAL;
                break;
            case STATE_NORMAL:
                current = (current % 4) + 1;
                WAIT_FOR(control_period);
            default: 
                break;
        }
    }
}

