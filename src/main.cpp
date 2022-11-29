#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "Constants.h"
#include "CLI/CLI.h"
#include "Utils/Utils.h"

WiFiUDP udp;
uint16_t local_port = 2390;

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

    udp.begin(local_port);

    print("Setup complete\n\n");
}

void loop() {
    char buffer[255];
    int packet_size = udp.parsePacket();
    if (packet_size) {
        print("Received packet of size %d\n", packet_size);
        print("From %s:%d\n", udp.remoteIP().toString().c_str(), udp.remotePort());

        // read the packet into packetBufffer
        int len = udp.read(buffer, 255);

        // null-terminate
        if (len > 0) {
            buffer[len] = 0;
        }

        print("Contents: %s\n", buffer);
    }
}