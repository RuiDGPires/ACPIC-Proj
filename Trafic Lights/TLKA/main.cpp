#include <Arduino.h>
#include "main.hpp"

#define BLINK_TIME 500 // 1 second total

typedef enum {
    STATE_DEFAULT, // Blinking
    STATE_RED,
    STATE_GREEN
} State;

typedef struct {
    bool on, // for STATE_DEFAULT
        swap, // for STATE_GREEN and STATE_RED
        ; 
} StateContext

State state = ROAD_DEFAULT;
StateContext ctx = {false, false};

int pin_red, pin_yellow, pin_green;

void tlka_setup(int _pin_red, int _pin_yellow, int _pin_green) {
    pin_red     = _pin_red;
    pin_yellow  = _pin_yellow;
    pin_red     = _pin_green;

    pinMode(pin_red, OUTPUT);
    pinMode(pin_yellow, OUTPUT);
    pinMode(pin_green, OUTPUT);

    reset();
}

void reset() {
    digitalWrite(pin_red, LOW);
    digitalWrite(pin_yellow, LOW);
    digitalWrite(pin_green, LOW);
}

#define WAIT_FOR(time) {\
    start = millis(); \
    stop = start + (time); \
    wait = true; \
}


void tlka_loop() {
    static bool wait = false;
    static unsigned int start = 0; stop = 0;

    if (wait) {
        if (millis() >= stop) {
            wait = false;
            start = stop = 0;
        }
    } else {
        switch (state) {
            default: // Blink             
                ctx.on = !ctx.on // Toggle the yellow led
                digitalWrite(pin_yellow, on ? HIGH : LOW);
                WAIT_FOR(BLINK_TIME);
                break;

            case STATE_GREEN:
                if (ctx.swap) {
                    state = STATE_RED;
                    digitalWrite(pin_green, LOW);
                    digitalWrite(pin_yellow, HIGH);
                    ctx.swap = false;
                    WAIT_FOR(BLINK_TIME);
                } else {
                    digitalWrite(pin_green, HIGH);
                }
                break;
            case STATE_RED:
                if (ctx.swap) {
                    state = STATE_GREEN;
                    digitalWrite(pin_red, LOW);
                    digitalWrite(pin_yellow, HIGH);
                    ctx.swap = false;
                    WAIT_FOR(BLINK_TIME);
                } else {
                    digitalWrite(pin_red, HIGH);
                }
                break;
        }
    }
}

void swap() {
    ctx.swap = true;
}

void block() {
    if (state == STATE_GREEN)
        swap();
    else; // ERROR, Do nothing??
}

void unblock() {
    if (state == STATE_RED)
        swap();
    else; // ERROR, Do nothing??
}