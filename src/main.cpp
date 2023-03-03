#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <FastLED.h>

#include "Constants.h"
#include "CLI/CLI.h"
#include "Timer/Timer.h"
#include "Utils/Utils.h"
#include "UDPClient/UDPClient.h"

#include "proto/ambilight.pb.h"

// Global LEDs array
CRGB leds[NUM_LEDS] = {0};

void setup() {
    pinMode(PIN_LED_STATUS, OUTPUT);
    
    Serial.begin(115200);  // debug serial terminal

    print("Initial safety delay\n");
    delay(500);  // power-up safety delay to avoid brown out

    print("Loading preferences\n");

    if (!check_prefs()) {
        print("Missing preferences! Starting cli.\n");
        start_cli();
    }

    // Wifi setup
    if (!connect_wifi()) {
        print("Failed to connect to wifi!");
    }

    // Setup LEDs
    print("Initializing LEDs on pin %d\n", PIN_LED_CONTROL);
    FastLED.addLeds<WS2812, PIN_LED_CONTROL, GRB>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.show();

    // Seed PRNG
    srand(time(NULL));

    print("Setup complete\n\n");
}

// void udp_config_callback(Message *message) {

// }

// void udp_data_callback() {

// }

void loop() {
    Message message;
    UDPClient udp = UDPClient(&message, WiFi.localIP());
    state_t state = udp.get_state();

    while (1) {
        if (state == START) {
            WiFi.setSleep(true);
        } else if (state == TO_DATA) {
            WiFi.setSleep(false);
        }
        state = udp.advance();
        if (state == DATA && message.type == MessageType_DATA) {
            print("Sequence number = %d\n", message.sequence_number);
            size_t to_copy = NUM_LEDS * 3;
            if (message.data.led_data.size < to_copy) {
                to_copy = message.data.led_data.size;
            }
            memcpy(leds, message.data.led_data.bytes, to_copy);
            FastLED.show();
        }
    }
}
