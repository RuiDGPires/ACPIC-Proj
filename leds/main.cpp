#include "Roundabout/main.hpp"
#include "Road/main.hpp"

void setup() {
    roundabout_setup();
    road_setup();
}

void loop() {
    roundabout_loop();
    road_loop();
}