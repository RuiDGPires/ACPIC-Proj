#include <Arduino.h>
#include "core.hpp"

#define BLINK_TIME 500 // 1 second total
#define YELLOW_TIME 500 // 1 second total

typedef struct {
    bool on = false, // for STATE_DEFAULT
        swap = false // for STATE_GREEN and STATE_RED
        ; 
} StateContext;

static TLKA_State state = TLKA_STATE_DEFAULT;
static StateContext ctx;

static int pin_red, pin_yellow, pin_green, pin_pedestrian_green, pin_pedestrian_red, pin_button;

static void reset() {
    digitalWrite(pin_red, LOW);
    digitalWrite(pin_yellow, LOW);
    digitalWrite(pin_green, LOW);
    digitalWrite(pin_pedestrian_green, LOW);
    digitalWrite(pin_pedestrian_red, LOW);
}

void tlka_setup(int _pin_red, int _pin_yellow, int _pin_green, int _pin_pedestrian_red, int _pin_pedestrian_green, int _pin_button) {
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

void tlka_loop() {
    static bool wait = false;
    static uint32_t start = 0, stop = 0;

    if (wait) {
        if (millis() >= stop) {
            wait = false;
            start = stop = 0;
            if (state == TLKA_STATE_G2R) {
                digitalWrite(pin_yellow, LOW);
                digitalWrite(pin_red, HIGH);
                state = TLKA_STATE_RED;
                // Pedestrian RED
                digitalWrite(pin_pedestrian_red, LOW);
                digitalWrite(pin_pedestrian_green, HIGH);
                //--
            } else if (state == TLKA_STATE_R2G) {
                digitalWrite(pin_yellow, LOW);
                digitalWrite(pin_green, HIGH);
                state = TLKA_STATE_GREEN;
            }
        }
    } else {
        switch (state) {
            default: // Blink             
                ctx.on = !ctx.on; // Toggle the yellow led
                digitalWrite(pin_yellow, ctx.on ? HIGH : LOW);
                WAIT_FOR(BLINK_TIME);
                break;

            case TLKA_STATE_GREEN:
                if (ctx.swap) {
                    state = TLKA_STATE_G2R;
                    digitalWrite(pin_green, LOW);
                    digitalWrite(pin_yellow, HIGH);
                    ctx.swap = false;
                    WAIT_FOR(YELLOW_TIME);
                }
                break;
            case TLKA_STATE_RED:
                if (ctx.swap) {
                    state = TLKA_STATE_R2G;
                    // Pedestrian RED
                    digitalWrite(pin_pedestrian_green, LOW);
                    digitalWrite(pin_pedestrian_red, HIGH);
                    //--
                    digitalWrite(pin_red, LOW);
                    digitalWrite(pin_yellow, HIGH);
                    ctx.swap = false;
                    WAIT_FOR(YELLOW_TIME);
                }
                break;
        }
    }
}

static void swap() {
    ctx.swap = true;
}

void tlka_block() {
    if (state == TLKA_STATE_GREEN)
        swap();
    else; // ERROR, Do nothing??
}

void tlka_unblock() {
    if (state == TLKA_STATE_RED)
        swap();
    else; // ERROR, Do nothing??
}

TLKA_State tlka_state() {
    return state;
}

void tlka_default() {
    reset();
    ctx.swap = false;
    ctx.on = false;
    state = TLKA_STATE_DEFAULT;
}
