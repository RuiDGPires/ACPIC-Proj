#include <Arduino.h>
#include "core.hpp"

#define BLINK_TIME 500 // 1 second total
#define YELLOW_TIME 500

typedef struct {
    bool on, // for STATE_DEFAULT
        swap // for STATE_GREEN and STATE_RED
        ; 
} StateContext;

static TLKB_State state = TLKB_STATE_DEFAULT;
static StateContext ctx = {false, false};
static int pin_red, pin_yellow, pin_green;

static bool fault_r;

bool tlkb_fault_r() {
    return fault_r;
}

static void reset() {
    digitalWrite(pin_red, LOW);
    digitalWrite(pin_yellow, LOW);
    digitalWrite(pin_green, LOW);
}

void tlkb_setup(int _pin_red, int _pin_yellow, int _pin_green) {
    pin_red     = _pin_red;
    pin_yellow  = _pin_yellow;
    pin_green     = _pin_green;

    pinMode(pin_red, OUTPUT);
    pinMode(pin_yellow, OUTPUT);
    pinMode(pin_green, OUTPUT);

    reset();
}


#define WAIT_FOR(time) {\
    start = millis(); \
    stop = start + (time); \
    wait = true; \
}

static bool red_pin_ok(int pin) {
    static const int error = 15, base = 963;
    unsigned int val = analogRead(pin);
    return  val >= base - error && val <= base + error;
}

void tlkb_loop() {
    static bool wait = false;
    static uint32_t start = 0, stop = 0;

    if (wait) {
        if (millis() >= stop) {
            wait = false;
            start = stop = 0;
            if (state == TLKB_STATE_G2R) {
                digitalWrite(pin_yellow, LOW);
                digitalWrite(pin_red, HIGH);
                fault_r = !red_pin_ok(pin_red);
                state = TLKB_STATE_RED;
            } else if (state == TLKB_STATE_R2G) {
                digitalWrite(pin_yellow, LOW);
                digitalWrite(pin_green, HIGH);
                state = TLKB_STATE_GREEN;
            }
        }
    } else {
        switch (state) {
            default: // Blink             
                ctx.on = !ctx.on; // Toggle the yellow led
                digitalWrite(pin_yellow, ctx.on ? HIGH : LOW);
                WAIT_FOR(BLINK_TIME);
                break;

            case TLKB_STATE_GREEN:
                if (ctx.swap) {
                    state = TLKB_STATE_G2R;
                    digitalWrite(pin_green, LOW);
                    digitalWrite(pin_yellow, HIGH);
                    ctx.swap = false;
                    WAIT_FOR(YELLOW_TIME);
                }
                break;
            case TLKB_STATE_RED:
                if (ctx.swap) {
                    state = TLKB_STATE_R2G;
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

void tlkb_block() {
    if (state == TLKB_STATE_GREEN)
        swap();
    else if (state == TLKB_STATE_DEFAULT) {
        state = TLKB_STATE_GREEN;
        swap();
    }
    else; // ERROR, Do nothing??
}

void tlkb_unblock() {
    if (state == TLKB_STATE_RED)
        swap();
    else if (state == TLKB_STATE_DEFAULT) {
        state = TLKB_STATE_RED;
        swap();
    }
    else; // ERROR, Do nothing??
}

void tlkb_default() {
    reset();
    ctx.swap = false;
    ctx.on = false;
    state = TLKB_STATE_DEFAULT;
}

TLKB_State tlkb_state() {
    return state;
}
