#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "Constants.h"
#include "CLI/CLI.h"
#include "Utils/Utils.h"

// Nanopb
#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "proto/ambilight.pb.h"

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
    // minimize latency
    WiFi.setSleep(false);

    udp.begin(local_port);

    print("Setup complete\n\n");
}

void loop() {
    uint8_t buffer[256];
    bool status;

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

        // print("Contents: %s\n", buffer);

        /* Allocate space for the decoded message. */
        Example message = Example_init_zero;
        
        /* Create a stream that reads from the buffer. */
        pb_istream_t stream = pb_istream_from_buffer(buffer, packet_size);
        
        /* Now we are ready to decode the message. */
        status = pb_decode(&stream, Example_fields, &message);
        
        /* Check for errors... */
        if (!status)
        {
            printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
        }
        
        /* Print the data contained in the message. */
        printf("type is %d!\n", (int)message.type);
        printf("name is %s!\n", (char *)message.name);

    }
}