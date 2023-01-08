#include <Wire.h>
#include <Arduino.h>
#include "coordination.hpp"

struct answer_t {
    bool request, ready;
    char buffer[10];
};

static struct answer_t answer;

void tl_coordination_loop() {

}

