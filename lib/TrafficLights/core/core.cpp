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
    TL_STATE_WAIT_ENTRY_UNBLOCKED,
    TL_STATE_WAIT_ROUND_UNBLOCKED,

    TL_STATE_NONE
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
    Operation op;
};

struct tl_response_t {
    char buffer[BUF_MAX]; 
    Operation op;
    bool ready, waiting;
};

static struct tl_response_t response;
static int state = TL_STATE_DEFAULT;
static char status = 0;


void tl_setup(int, int, int, int, int, int) {
    tlkb_setup(N3(0)); // TODO
    tlka_setup(N6(0)); // TODO
}

static char checksum(const char buffer[], int integrity_index) {
    // Checksum
    char sum = 0;
    for (int i = 0; i < integrity_index; i++)
        sum += buffer[i];

    return sum;
}


void build_response() {
    response.buffer[0] = ENTRY_NUMBER;
    response.buffer[1] = response.op;
    response.buffer[2] = 0;

    int integrity = 3;

    if (response.op == OP_STATUS) {
        response.buffer[3] = status; 
        integrity++;
    }
   
    response.buffer[integrity] = checksum(response.buffer, integrity);
    response.waiting = false;
    response.ready = true;
}

void tl_loop() {
    switch (state) {
        case TL_STATE_WAIT_ENTRY_BLOCKED:
            if (tlka_state() == TLKA_STATE_RED) {
                tlkb_unblock();  
                state = TL_STATE_WAIT_ROUND_UNBLOCKED;
            }
            break;
        case TL_STATE_WAIT_ROUND_BLOCKED:
            if (tlkb_state() == TLKB_STATE_RED) {
                tlka_unblock();  
                state = TL_STATE_WAIT_ENTRY_UNBLOCKED;
            }
            break;
        case TL_STATE_WAIT_ENTRY_UNBLOCKED:
            if (tlka_state() == TLKA_STATE_GREEN) {
                state = TL_STATE_REST;
                if (response.waiting && response.op == OP_ACK) {
                    build_response();
                }
            }
            break;
        case TL_STATE_WAIT_ROUND_UNBLOCKED:
            if (tlkb_state() == TLKB_STATE_GREEN) {
                state = TL_STATE_REST;
                if (response.waiting && response.op == OP_ACK) {
                    build_response();
                }
            }
            break;
        default:
            break;
    }
    tlka_loop();
    tlkb_loop();
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

void tl_message(const char buffer[]) {
    struct message_t msg;
    // UNSAFE | NO CHECKS FOR THE BUFFER LENGTH
    msg.sender         = (unsigned int) buffer[0];
    msg.op             = (Operation) buffer[1];
    msg.destination    = (unsigned int) buffer[2];
    
    const char sum = checksum(buffer, 3);

    if (sum != buffer[3] || msg.destination != ENTRY_NUMBER) // error?
        return; // TODO : ?
   
    response.ready = false;
    response.waiting = true;

    switch (msg.op) {
        case OP_RED:
            tlka_block();
            state = TL_STATE_WAIT_ENTRY_BLOCKED;
            response.op = OP_ACK;
            break;
        case OP_GREEN:
            tlkb_block();
            state = TL_STATE_WAIT_ROUND_BLOCKED;
            response.op = OP_ACK;
            break;

        case OP_PING:
            response.op = OP_STATUS;
            build_response();
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
        response.ready = false;
        return true;
    }
    
    return false;
}
