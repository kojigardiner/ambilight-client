#include <Arduino.h>
#include <WiFi.h>

#include "Constants.h"
#include "CLI/CLI.h"
#include "Utils/Utils.h"

void setup() {
    pinMode(PIN_LED_STATUS, OUTPUT);
    
    Serial.begin(115200);  // debug serial terminal

    print("Initial safety delay\n");
    delay(500);  // power-up safety delay to avoid brown out

    // disableCore0WDT();  // hack to avoid various kernel panics

    print("Loading preferences\n");

    if (!check_prefs()) {
        print("Missing preferences! Starting cli.\n");
        start_cli();
    }

    // Wifi setup
    if (!connect_wifi()) {
        print("Failed to connect to wifi!");
    }

    print("Setup complete\n\n");
}

void loop() {
  // put your main code here, to run repeatedly:
}