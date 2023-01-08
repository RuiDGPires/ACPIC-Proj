#include <Arduino.h>
#include <Wire.h>
#include "TLKA/main.hpp"
#include "TLKB/main.hpp"
#include "main.hpp"

#ifndef __MAIN__
    void setup() {
        tl_setup();
    }

    void loop() {
        tl_loop();
    }
#endif

void tl_setup() {
    int entryNumber = getEntryNumber();
    Wire.begin(entryNumber);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    tlka_setup(1, 2, 3);
    tlkb_setup();
}

void tl_loop() {
    tlka_loop();
    tlkb_loop();
}

typedef enum {
    OP_RED = 0,
    OP_GREEN,
    OP_OFF,
    OP_PING,
    OP_ACK,
    OP_STATUS
} Operation;

// TODO : ASK IF IT IS LEFT TO RIGHT OR RIGHT TO LEFT
enum {
    TIMER_ACTIVTED          = 0x1 << 1,  // BUTTON PRESSED
    GREEN_FAILING           = 0x1 << 2,
    YELLOW_FAILING          = 0x1 << 3,
    RED_FAILING             = 0x1 << 4,
    PEDEST_GREEN_FAILING    = 0x1 << 5,
    PEDEST_YELLOW_FAILING   = 0x1 << 6,
    PEDEST_RED_FAILING      = 0x1 << 7,
};

struct message_t {
    unsigned int sender, destination;
    unsigned int info;
    Operation op;
}

void parse_message(const char buffer[], struct message_t *msg) {
    // UNSAFE | NO CHECKS FOR THE BUFFER LENGTH
    msg->sender         = (unsigned int) buffer[0];
    msg->op             = (Operation) buffer[1];
    msg->destination    = (unsigned int) buffer[2];

    if (msg->op == OP_STATUS)
        msg->info = (unsigned int) buffer[3];
}

void receiveEvent() {

}

void requestEvent() {

}
