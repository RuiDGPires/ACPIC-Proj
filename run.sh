#!/bin/sh

env="all"

if [ "$1" = "tl" ] || [ "$1" = "TrafficLights" ]; then
    env="trafficlights"
fi

if [ "$1" = "ct" ] ||[ "$1" = "ctr" ] || [ "$1" = "Controller" ]; then
    env="controller"
fi

pio run --target upload -e $env
