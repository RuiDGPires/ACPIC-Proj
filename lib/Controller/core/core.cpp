#include <Arduino.h>
#include <Wire.h>
#include "core.hpp"
#include "inputs.hpp"

#ifdef __TL__ // If the Traffic Lights and the Controller are implement on the same device...
#define ENTRY_NUMBER 1
#include <TrafficLights.hpp>
#endif

#define BUF_MAX 5
#define POT_MIN 0
#define POT_MAX 1023
#define PERIOD_MIN 2000
#define PERIOD_MAX 15000
#define BLINK_PERIOD 250

#define WAIT_FOR(time) {\
    start = millis(); \
    stop = start + (time); \
    wait = true; \
}

typedef enum {
    STATE_OFF,
    STATE_ENTRY,
    STATE_NORMAL,
    STATE_FAULT,
} State;

static int led_pin;
static struct inputs inputs;
static State state = STATE_OFF;
static int current = 1;

// Wait timers
static bool wait = false;
static uint32_t start = 0, stop = 0;
static bool timer_once = false;

static char checksum(const char buffer[], int integrity_index) { // Calculate the checksum of a buffer
    // Checksum
    char sum = 0;
    for (int i = 0; i < integrity_index; i++)
        sum += buffer[i];

    return sum;
}

static void timer_activated() { // Check if pedestrian button was pressed
    if (state == STATE_NORMAL && !timer_once) { // Skip if time has already been shortened
        uint32_t now = millis();
        stop = start + (stop - now) / 2; // Halve the remaining time
        timer_once = true;
        Serial.println("Timer Activated");
    }
}

static void print_msg(const char buffer[]) { // Print a messag
    Serial.print(  String((int)buffer[0]) + "|");
    Serial.print(  String((int)buffer[1]) + "|");
    Serial.print(  String((int)buffer[2]) + "|");
    if (buffer[1] == OP_STATUS) {
        Serial.print(String((int)buffer[3]) + "|");
        Serial.println(String((int)buffer[4]));
    } else {
        Serial.println(String((int)buffer[3]));
    }
}

static char faults[4] = {0, 0, 0, 0};
static char faults_new[4] = {0, 0, 0, 0};

static void ct_send_message(int to, Operation op) { // Sends a message and waits a resposne
    if (to > N_ENTRIES)
        return;
    char buffer[BUF_MAX];

    buffer[0] = 0;
    buffer[1] = (char) op;
    buffer[2] = (char) to;
    buffer[3] = checksum(buffer, 3);

    Serial.print("Sending: ");
    print_msg(buffer);

    int size = op == OP_PING ? 5 : 4;
#ifdef __TL__ // Check if message is for this device (only if Traffic Lights are also implemented)
    if (to == ENTRY_NUMBER) {
        tl_message(buffer);
        tl_response(buffer);
        goto parse_response;
    }
#endif

    Wire.beginTransmission(to);
    for (int i = 0; i < 4; i++)
        Wire.write(buffer[i]);
    Wire.endTransmission();

    bool on = true;
    uint32_t next = 0;
    while (true) { // Wait for acknowledge

        uint32_t now = millis(); // Blink
        if (now >= next) {
            on = !on;
            digitalWrite(led_pin, on ? HIGH : LOW);
            next = now + BLINK_PERIOD;
        }

        Wire.requestFrom(to, size);
        while (size > Wire.available()) {
            uint32_t now = millis(); // Blink
            if (now >= next) {
                on = !on;
                digitalWrite(led_pin, on ? HIGH : LOW);
                next = now + BLINK_PERIOD;
            }
        }

        for (int i = 0; i < size; i++)
            buffer[i] = Wire.read();

        // If the response is correct, break from the loop
        if (op == OP_PING && buffer[1] == OP_STATUS)
            break;
        else if (buffer[1] == OP_ACK)
            break;
    }

    // Return to normal
    digitalWrite(led_pin, HIGH);

    // [DEBUG] Print response
    Serial.print("Response: ");
    print_msg(buffer);

parse_response:;
    if (buffer[1] == OP_STATUS) {
        char info = buffer[3];
        const char any_fault = PEDEST_GREEN_FAILING | PEDEST_YELLOW_FAILING | PEDEST_RED_FAILING | GREEN_FAILING | YELLOW_FAILING | RED_FAILING;

        faults_new[to] |= info & any_fault;
        if (faults[to] & faults_new[to]) {
            // FAILURE; TURN EVERYTHING OFF
            ct_send_message(1, OP_OFF);
            ct_send_message(2, OP_OFF);
            ct_send_message(3, OP_OFF);
            ct_send_message(4, OP_OFF);
            state = STATE_FAULT;
        }

        // If pedestrian button was activated
        if (info & TIMER_ACTIVATED)
            timer_activated();
    }
}

static void clear_faults() {
    faults[0] = faults_new[0];
    faults[1] = faults_new[1];
    faults[2] = faults_new[2];
    faults[3] = faults_new[3];
    faults_new[0] = 0;
    faults_new[1] = 0;
    faults_new[2] = 0;
    faults_new[3] = 0;
}

static void toggleOnOff() { // Toggle the controller on and off
    if (state == STATE_OFF) { // Turn On
        state = STATE_ENTRY;
    } else {
        ct_send_message(1, OP_OFF);
        ct_send_message(2, OP_OFF);
        ct_send_message(3, OP_OFF);
        ct_send_message(4, OP_OFF);
        digitalWrite(led_pin, LOW);
        state = STATE_OFF;
    }
}

void ct_setup(int _led_pin, int _potetiometer_pin, int _button_pin) { // Setup controller
    led_pin = _led_pin;
    Wire.begin();
    inputs_setup(_button_pin, _potetiometer_pin);
    pinMode(led_pin, OUTPUT);
}

static void ping(unsigned int time) { // Ping the current entry in intervals of <time>
    static uint32_t start = 0, stop = 0;
    static bool wait = false;

    if (wait) {
        if (millis() >= stop) {
            wait = false;
            start = stop = 0;
        }
    } else {
        if (state == STATE_NORMAL) {// Ping the current leds to check if the timer button has been pressed
            ct_send_message(current, OP_PING);
            WAIT_FOR(time);
        }
    }
}

void ct_loop() {
    inputs_check(&inputs);

    if (inputs.button) { // Check button
        inputs.button = false;
        toggleOnOff();
    }

    if (wait) {
        if (millis() >= stop) {
            wait = false;
            start = stop = 0;
        }

        if (state == STATE_NORMAL) // Ping the current led
            ping(200);

    } else {
        uint32_t time = map(inputs.pot, POT_MIN, POT_MAX, PERIOD_MIN, PERIOD_MAX);
        switch (state) {
            // Entering the normal state
            case STATE_ENTRY:
                digitalWrite(led_pin, HIGH);
                ct_send_message(2, OP_RED);
                ct_send_message(3, OP_RED);
                ct_send_message(4, OP_RED);
                //--
                current = 1;

                // Double clear faults to clear both buffers;
                clear_faults();
                clear_faults();

                ct_send_message(current, OP_GREEN);
                state = STATE_NORMAL;
                WAIT_FOR(time);
                break;
            case STATE_NORMAL:
                ct_send_message(current, OP_RED);
                current = (current % 4) + 1;

                if (current == 1) // New cycle
                    clear_faults();
                ct_send_message(current, OP_GREEN);
                timer_once = false;
                WAIT_FOR(time);
                break;
            default: 
                break;
        }
    }
}

