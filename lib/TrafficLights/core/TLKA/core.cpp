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
static bool fault_pr, fault_r;

bool tlka_fault_pr(){
    return fault_pr;
}

bool tlka_fault_r() {
    return fault_r;
}

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
    pin_green     = _pin_green;
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

static bool red_pin_ok(int pin) {
    static const int error = 15, base = 963;
    unsigned int val = analogRead(pin);
    return  val >= base - error && val <= base + error;
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
                
                fault_r = !red_pin_ok(pin_red);

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
                    fault_pr = !red_pin_ok(pin_pedestrian_red);
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
    else if (state == TLKA_STATE_DEFAULT) {
        state = TLKA_STATE_GREEN;
        swap();
    }
}

void tlka_unblock() {
    if (state == TLKA_STATE_RED)
        swap();
    else if (state == TLKA_STATE_DEFAULT) {
        state = TLKA_STATE_RED;
        swap();
    }
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

bool tlka_check_button() { // check if button is released and debounced
    bool ret = false;
  static int buttonState = LOW;         	// the current reading from the input pin
  static int lastButtonState = LOW;   // the previous reading from the input pin

    // the following variables are unsigned long's because the time, measured in miliseconds,
    // will quickly become a bigger number than can be stored in an int.
  static unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
  static unsigned long debounceDelay = 20;  // the debounce time; increase if the output flickers

  // read the state of the switch into a local variable:
  int reading = digitalRead(pin_button);
  //Serial.println(reading);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
	// reset the debouncing timer
	lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
	// whatever the reading is at, it's been there for longer
	// than the debounce delay, so take it as the actual current state:
	// if the button state has changed:
	if (reading != buttonState) {
  	buttonState = reading;

  	// only toggle the LED if the new button state is LOW
  	if (buttonState == LOW) {
    	ret = true;
        Serial.println("Button pressed!");
  	}
	}
  }
  lastButtonState = reading;
  return ret;
}
