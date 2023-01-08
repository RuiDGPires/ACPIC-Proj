#include <Arduino.h>
#include "core.hpp"

#define BLINK_TIME 500 // 1 second total

enum {
    STATE_DEFAULT, // Blinking
    STATE_RED,
    STATE_GREEN
};

typedef struct {
    bool on = false, // for STATE_DEFAULT
        swap = false // for STATE_GREEN and STATE_RED
        ; 
} StateContext;

static int state = STATE_DEFAULT;
static StateContext ctx;

static int pin_red, pin_yellow, pin_green, pin_pedestrian_green, pin_pedestrian_red, pin_button;

static void reset() {
    digitalWrite(pin_red, LOW);
    digitalWrite(pin_yellow, LOW);
    digitalWrite(pin_green, LOW);
    digitalWrite(pin_pedestrian_green, LOW);
    digitalWrite(pin_pedestrian_red, LOW);
}

void tlkb_setup(int _pin_red, int _pin_yellow, int _pin_green, int _pin_pedestrian_red, int _pin_pedestrian_green, int _pin_button) {
    Serial.begin(4800);
    pin_red     = _pin_red;
    pin_yellow  = _pin_yellow;
    pin_red     = _pin_green;
    pin_pedestrian_green = _pin_pedestrian_green;
    pin_pedestrian_red = _pin_pedestrian_red;
    pin_button = _pin_button;

    pinMode(pin_red, OUTPUT);
    pinMode(pin_yellow, OUTPUT);
    pinMode(pin_green, OUTPUT);
    pinMode(pin_pedestrian_green, OUTPUT);
    pinMode(pin_pedestrian_red, OUTPUT);
    pinMode(pin_button, INPUT);

    reset();
}

#define WAIT_FOR(time) {\
    start = millis(); \
    stop = start + (time); \
    wait = true; \
}


void tlkb_loop() {
    static bool wait = false;
    static uint32_t start = 0, stop = 0;

    if (wait) {
        if (millis() >= stop) {
            wait = false;
            start = stop = 0;
        }
    } else {
        switch (state) {
            default: // Blink             
                ctx.on = !ctx.on; // Toggle the yellow led
                digitalWrite(pin_yellow, ctx.on ? HIGH : LOW);
                WAIT_FOR(BLINK_TIME);
                break;

            case STATE_GREEN:
                if (ctx.swap) {
                    digitalWrite(pin_green, LOW);
                    state = STATE_RED;
                    digitalWrite(pin_yellow, HIGH);
                    ctx.swap = false;
                    WAIT_FOR(BLINK_TIME);
                } else {
                    digitalWrite(pin_green, HIGH);
                }
                break;
            case STATE_RED:
                if (ctx.swap) {
                    digitalWrite(pin_red, LOW);
                    state = STATE_GREEN;
                    digitalWrite(pin_yellow, HIGH);
                    WAIT_FOR(BLINK_TIME);
                } else {
                    digitalWrite(pin_red, HIGH);
                }
                break;
        }
    }
}

static void swap() {
    ctx.swap = true;
}

static void status() {

}
