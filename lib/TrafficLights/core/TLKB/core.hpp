#ifndef __TLKB_CORE_HPP__
#define __TLKB_CORE_HPP__

typedef enum {
    TLKB_STATE_DEFAULT, // Blinking
    TLKB_STATE_G2R,
    TLKB_STATE_RED,
    TLKB_STATE_R2G,
    TLKB_STATE_GREEN
} TLKB_State;

void tlkb_setup(int, int, int);
void tlkb_loop();
void tlkb_block();
void tlkb_unblock();
TLKB_State tlkb_state();
void tlkb_default();
bool tlkb_fault_r();

#endif
