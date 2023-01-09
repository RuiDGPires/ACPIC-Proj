#!/bin/sh

env="all"

if [ "$1" = "tl" ] || [ "$1" = "TrafficLights" ]; then
    env="trafficlights"
fi

if [ "$1" = "ct" ] ||[ "$1" = "ctr" ] || [ "$1" = "Controller" ]; then
    env="controller"
fi

if [ "$1" = "monitor" ]; then
    if [ "$2" = "ct" ]; then
        pio device monitor --port /dev/ttyACM1
    fi
    if [ "$2" = "tl" ]; then
        pio device monitor --port /dev/ttyACM0
    fi
else
    pio run --target upload -e $env
fi

