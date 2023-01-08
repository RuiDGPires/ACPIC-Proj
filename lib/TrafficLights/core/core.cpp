#include <Arduino.h>
#include <Wire.h>
#include "TLKA/core.hpp"
#include "TLKB/core.hpp"
#include "core.hpp"

#define BUF_MAX 5

enum {
    TL_STATE_DEFAULT,
    TL_STATE_REST,
    TL_STATE_WAIT_ENTRY_BLOCKED,
    TL_STATE_WAIT_ROUND_BLOCKED,

    TL_STATE_NONE
};

static int state = TL_STATE_DEFAULT;

void tl_setup(int, int, int, int, int, int) {
    tlka_setup(N3(0)); // TODO
    tlkb_setup(N6(0)); // TODO
}

void tl_loop() {
    switch (state) {
        case TL_STATE_WAIT_ENTRY_BLOCKED:
            if (tlka_state() == TLKA_STATE_RED) {
                state = TL_STATE_REST;
                tlkb_unblock();  
            }
            break;
        case TL_STATE_WAIT_ROUND_BLOCKED:
            if (tlkb_state() == TLKB_STATE_RED) {
                state = TL_STATE_REST;
                tlka_unblock();  
            }
            break;
        default:
            break;
    }
    tlka_loop();
    tlkb_loop();
}

char checksum(const char buffer[], int integrity_index) {
    // Checksum
    char sum = 0;
    for (int i = 0; i < integrity_index; i++)
        sum += buffer[i];

    return sum;
}

// TODO : ASK IF IT IS LEFT TO RIGHT OR RIGHT TO LEFT
enum {
    TIMER_ACTIVATED         = 0x1 << 1,  // BUTTON PRESSED
    GREEN_FAILING           = 0x1 << 2,
    YELLOW_FAILING          = 0x1 << 3,
    RED_FAILING             = 0x1 << 4,
    PEDEST_GREEN_FAILING    = 0x1 << 5,
    PEDEST_YELLOW_FAILING   = 0x1 << 6,
    PEDEST_RED_FAILING      = 0x1 << 7,
};

typedef enum {
    OP_RED = 0,
    OP_GREEN,
    OP_OFF,
    OP_PING,
    OP_ACK,
    OP_STATUS
} Operation;

struct message_t {
    unsigned int sender, destination;
    unsigned int info;
    Operation op;
};

struct tl_response_t {
    char buffer[BUF_MAX]; 
    bool ready;
};

static struct tl_response_t response;

void tl_message(const char buffer[]) {
    struct message_t msg;
    // UNSAFE | NO CHECKS FOR THE BUFFER LENGTH
    msg.sender         = (unsigned int) buffer[0];
    msg.op             = (Operation) buffer[1];
    msg.destination    = (unsigned int) buffer[2];
    
    if (msg.op == OP_STATUS)
        msg.info = (unsigned int) buffer[3];

    const int integrity = msg.op == OP_STATUS ? 4 : 3;
    const char sum = checksum(buffer, integrity);

    if (sum != buffer[integrity] || msg.destination != ENTRY_NUMBER) // error?
        return; // TODO : ?
   
    response.ready = false;

    switch (msg.op) {
        case OP_RED:
            tlka_block();
            state = TL_STATE_WAIT_ENTRY_BLOCKED;
            break;
        case OP_GREEN:
            tlkb_block();
            state = TL_STATE_WAIT_ROUND_BLOCKED;
            break;

        default:
            break;
    }
}

bool tl_response(char buffer[]) {
    if (response.ready) {
        for (int i = 0; i < BUF_MAX; i++) {
            buffer[i] = response.buffer[i];
        }

        return true;
    }
    
    return false;
}
