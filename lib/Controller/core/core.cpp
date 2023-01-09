#include <Arduino.h>
#include <Wire.h>
#include "core.hpp"
#include "inputs.hpp"

#ifdef __TL__ // If the Traffic Lights and the Controller are implement on the same device...
#define ENTRY_NUMBER 1
#include <TrafficLights.hpp>
#endif

#define BUF_MAX 5
#define POT_MIN 0
#define POT_MAX 255 
#define PERIOD_MIN 2
#define PERIOD_MAX 15

typedef enum {
    STATE_OFF,
    STATE_ENTRY,
    STATE_NORMAL,
    STATE_FAULT,
} State;

static int potetiometer_pin, button_pin, led_pin;
static struct inputs inputs;
static State state = STATE_OFF;
static int current = 1;

// Wait timers
static bool wait = false;
static uint32_t start = 0, stop = 0;
static bool timer_once = false;

static char checksum(const char buffer[], int integrity_index) {
    // Checksum
    char sum = 0;
    for (int i = 0; i < integrity_index; i++)
        sum += buffer[i];

    return sum;
}

static void timer_activated() {
    if (state == STATE_NORMAL && wait && !timer_once) { // Skip if time has already been shortened
        uint32_t now = millis();
        stop = start + (stop - now) / 2; // Halve the remaining time
        timer_once = true;
    }
}

static void ct_send_message(int to, Operation op) {
    char buffer[BUF_MAX];

    buffer[0] = 0;
    buffer[1] = (char) op;
    buffer[2] = (char) to;
    buffer[3] = checksum(buffer, 3);

    int size = op == OP_PING ? 5 : 4;
#ifdef __TL__ // Check if message is for this device
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
        char info = buffer[3];

        if (info & (PEDEST_GREEN_FAILING | PEDEST_YELLOW_FAILING | PEDEST_RED_FAILING | GREEN_FAILING | YELLOW_FAILING | RED_FAILING)) {
            ct_send_message(1, OP_OFF);
            ct_send_message(2, OP_OFF);
            ct_send_message(3, OP_OFF);
            ct_send_message(4, OP_OFF);
            state = STATE_FAULT;
        }

        if (info & TIMER_ACTIVATED)
            timer_activated();
    }
}

static void toggleOnOff() {
    if (state == STATE_OFF) { // Turn On
        state = STATE_ENTRY;
    } else {
        ct_send_message(1, OP_OFF);
        ct_send_message(2, OP_OFF);
        ct_send_message(3, OP_OFF);
        ct_send_message(4, OP_OFF);
        digitalWrite(led_pin, LOW);
        state = STATE_OFF;
    }
}

#define WAIT_FOR(time) {\
    start = millis(); \
    stop = start + (time); \
    wait = true; \
}

void ct_setup(int _led_pin, int _potetiometer_pin, int _button_pin) {
    led_pin = _led_pin;
    potetiometer_pin = _potetiometer_pin;
    button_pin = _button_pin;
    inputs_setup(button_pin, potetiometer_pin);
    pinMode(led_pin, OUTPUT);
}

void ct_loop() {
    inputs_check(&inputs);

    if (inputs.button) {
        inputs.button = false;
        toggleOnOff();
    }

    if (wait) {
        if (millis() >= stop) {
            wait = false;
            start = stop = 0;
        }
        if (state == STATE_NORMAL) // Ping the current leds to check if the timer button has been pressed
            ct_send_message(current, OP_PING);
    } else {
        switch (state) {
            case STATE_ENTRY:
                digitalWrite(led_pin, HIGH);
                ct_send_message(2, OP_RED);
                ct_send_message(3, OP_RED);
                ct_send_message(4, OP_RED);
                //--
                current = 1;
                ct_send_message(current, OP_GREEN);
                state = STATE_NORMAL;
                break;
            case STATE_NORMAL:
                ct_send_message(current, OP_RED);
                current = (current % 4) + 1;
                ct_send_message(current, OP_GREEN);
                timer_once = false;
                WAIT_FOR(PERIOD_MIN + (PERIOD_MAX - PERIOD_MIN) * (inputs.pot - POT_MIN)/(POT_MAX - POT_MIN));
                break;
            default: 
                break;
        }
    }
}

