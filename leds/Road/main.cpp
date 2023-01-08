#include <Arduino.h>
#include "main.hpp"

#define BLINK_TIME 500 // 1 second total

typedef enum {
    STATE_DEFAULT, // Blinking
    STATE_RED,
    STATE_YELLOW_R2G,
    STATE_YELLOW_G2R,
    STATE_GREEN
} State;

typedef struct {
    bool on; // for STATE_DEFAULT
} StateContext

State state = ROAD_DEFAULT;
StateContext ctx;

int pin_red, pin_yellow, pin_green;

void road_setup(int _pin_red, int _pin_yellow, int _pin_green) {
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
    digitalWrite(pin_red, LOW);
}

void road_loop() {
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
                start = millis();
                stop = start + BLINK_TIME;
                wait = true;
        }
    }
}

void block() {

}

void unblock() {

}

void status() {

}