#ifndef __INPUTS_HPP__
#define __INPUTS_HPP__

struct inputs {
    bool button;
    unsigned int pot;
};

void inputs_setup(int button_pin, int pot_pin);
void inputs_check(struct inputs *inputs);

#endif
