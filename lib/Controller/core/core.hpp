#ifndef __CT_CORE__
#define __CT_CORE__

void ct_setup(int);
void ct_loop();

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

#endif
