#include <Arduino.h>
#include "inputs.hpp"

static int button_pin, pot_pin;

void inputs_setup(int _button_pin, int _pot_pin) {
  button_pin = _button_pin;
  pot_pin = _pot_pin;
    Serial.println("Button pin set up: " + String(button_pin));
    pinMode(button_pin, INPUT);
    pinMode(pot_pin, INPUT);
}

static bool check_button() { // check if button is released and debounced
    bool ret = false;
  static int buttonState = LOW;         	// the current reading from the input pin
  static int lastButtonState = LOW;   // the previous reading from the input pin

    // the following variables are unsigned long's because the time, measured in miliseconds,
    // will quickly become a bigger number than can be stored in an int.
  static unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
  static unsigned long debounceDelay = 20;  // the debounce time; increase if the output flickers

  // read the state of the switch into a local variable:
  int reading = digitalRead(button_pin);
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

// Check inputs
void inputs_check(struct inputs *inputs) {
  if (check_button())
    inputs->button = true;
  inputs->pot = analogRead(pot_pin);
}
