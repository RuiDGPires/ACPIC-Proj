#ifndef __TLKA_CORE_H__
#define __TLKA_CORE_H__

typedef enum {
    TLKA_STATE_DEFAULT, // Blinking
    TLKA_STATE_G2R,
    TLKA_STATE_RED,
    TLKA_STATE_R2G,
    TLKA_STATE_GREEN
} TLKA_State;

void tlka_setup(int, int, int, int, int, int);
void tlka_loop();

void tlka_block();
void tlka_unblock();

TLKA_State tlka_state();
void tlka_default();

#endif
