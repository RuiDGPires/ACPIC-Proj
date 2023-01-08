#include <Arduino.h>
#include <Wire.h>
#include "TLKA/core.hpp"
#include "TLKB/core.hpp"
#include "core.hpp"

// ------
//  UTIL
// ------

#define N2(n) n, n+1
#define N3(n) N2(n), n+2
#define N4(n) N3(n), n+3
#define N5(n) N4(n), n+4
#define N6(n) N5(n), n+5
#define N7(n) N6(n), n+6
#define N8(n) N7(n), n+7
#define N9(n) N8(n), n+8
#define N10(n) N9(n), n+9
#define N11(n) N10(n), n+10

void tl_setup(int, int, int, int, int, int) {
    tlka_setup(N3(0)); // TODO
    tlkb_setup(N6(0)); // TODO
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
};

char checksum(const char buffer[], int integrity_index) {
    // Checksum
    char sum = 0;
    for (int i = 0; i < integrity_index; i++)
        sum += buffer[i];

    return sum;
}

void parse_message(const char buffer[], struct message_t *msg) {
    // UNSAFE | NO CHECKS FOR THE BUFFER LENGTH
    msg->sender         = (unsigned int) buffer[0];
    msg->op             = (Operation) buffer[1];
    msg->destination    = (unsigned int) buffer[2];
    
    if (msg->op == OP_STATUS)
        msg->info = (unsigned int) buffer[3];

    const int integrity = msg->op == OP_STATUS ? 4 : 3;
    const char sum = checksum(buffer, integrity);
    if (sum != buffer[integrity])
        return; // TODO : ?
}

