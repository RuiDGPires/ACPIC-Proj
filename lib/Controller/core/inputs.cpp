#include <Arduino.h>
#include "inputs.hpp"

void inputs_setup(int button_pin, int pot_pin) {
    pinMode(button_pin, INPUT);
    pinMode(pot_pin, INPUT);
}

void inputs_check(struct inputs *inputs) {
    
}
