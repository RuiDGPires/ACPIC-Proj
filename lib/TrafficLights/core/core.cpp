#ifndef __TL_CORE_CPP__
#define __TL_CORE_CPP__

#include <Arduino.h>
#include <Wire.h>
#include "TLKA/core.hpp"
#include "TLKB/core.hpp"
#include "core.hpp"

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

enum {
    TL_STATE_DEFAULT,
    TL_STATE_REST,
    TL_STATE_WAIT_ENTRY_BLOCKED,
    TL_STATE_WAIT_ROUND_BLOCKED,
    TL_STATE_WAIT_ENTRY_UNBLOCKED,
    TL_STATE_WAIT_ROUND_UNBLOCKED,

    TL_STATE_NONE
};

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

void tl_setup(int ar, int ay, int ag, int pb, int pr, int pg, int br, int by, int bg) {
    tlkb_setup(br, by, bg);
    tlka_setup(ar, ay, ag, pr, pg, pb);
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
        status &= ~((unsigned char) TIMER_ACTIVATED);
        integrity++;
    }
   
    response.buffer[integrity] = checksum(response.buffer, integrity);
    response.waiting = false;
    response.ready = true;
}

void tl_loop() {
    if (tlka_fault_r()) {
        status |= RED_FAILING;
    }
    if (tlka_fault_pr()) {
        status |= PEDEST_RED_FAILING;
    }
    if (tlkb_fault_r()) {
        status |= RED_FAILING;
    }
    if (tlka_check_button()) {
        status |= TIMER_ACTIVATED;
    }

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
        case OP_OFF:
            tlka_default();
            tlkb_default();
            state = TL_STATE_DEFAULT;
            response.op = OP_ACK;
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
#endif
