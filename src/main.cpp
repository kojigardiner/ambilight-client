#include <Arduino.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <time.h>

#include "Constants.h"
#include "CLI/CLI.h"
#include "Utils/Utils.h"

// Nanopb
#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "proto/ambilight.pb.h"

// Function Prototypes
bool parse_pb(WiFiUDP *udp, int packet_size, Message *message);
bool create_config_pb(uint8_t *buffer, size_t *message_length, IPAddress local_ip, uint16_t local_port);

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
    
    // Minimize latency in exchange for using more power
    WiFi.setSleep(false);

    // Seed PRNG
    srand(time(NULL));

    print("Setup complete\n\n");
}

void loop() {
    WiFiUDP udp;
    
    IPAddress server_ip;
    uint16_t server_port;

    IPAddress local_ip;
    uint16_t local_port;

    // For sending encoded messages
    uint8_t encoded_message[MAX_MESSAGE_BYTES];
    size_t message_length;

    uint32_t sequence_number = -1;

    state_t state = START;

    for (;;) {
        switch (state) {
            /*
                State: START
                Behavior: Setup UDP port.
                Transition: Once setup, transition to IDLE.
             */
            case START: {
                local_ip = WiFi.localIP();
                local_port = UDP_BROADCAST_PORT;
                print("Starting UDP at %s:%d\n", local_ip.toString().c_str(), local_port);
                udp.begin(local_port);

                print("Transition: START ==> IDLE\n");
                state = IDLE;
                break;
            }
            /*
                State: IDLE
                Behavior: Listen for discovery message. Send config packets every 
                IDLE_CONFIG_MS
                
                Transition: On receipt of discovery message, store server's IP/port, start
                timer for DISCOVERY_MS, and transition to DISCOVERY state.
            */
            case IDLE: {
                int packet_size = udp.parsePacket();
                if (packet_size) {
                    print("%d bytes from %s:%d\n", packet_size, udp.remoteIP().toString().c_str(), udp.remotePort());

                    Message decoded_message = Message_init_zero;    // reset the message
                    if (parse_pb(&udp, packet_size, &decoded_message)) {
                        if (decoded_message.type == MessageType_DISCOVERY) {
                            print("Received discovery message!\n");

                            server_ip = udp.remoteIP();
                            server_port = udp.remotePort();
                            print("Setting server as %s:%d\n", server_ip.toString().c_str(), server_port);
                            
                            udp.stop();
                            local_port = (rand() % (MAX_UDP_PORT - MIN_UDP_PORT)) + MIN_UDP_PORT;
                            print("Restarting UDP service on port %d\n", local_port);
                            udp.begin(local_port);

                            // TODO: Start discovery timer
                            print("Transition: IDLE ==> DISCOVERY\n");
                            state = DISCOVERY;
                        }
                    }
                }
                break;
            }
            /*
                State: DISCOVERY

                Behavior: Send config packets every DISCOVERY_CONFIG_MS.

                Transition: On receipt of ACK, transition to DATA state. If no ACK is
                received before timeout, transition to IDLE state.
            */
            case DISCOVERY: {
                if (!udp.beginPacket(server_ip, server_port)) {
                    print("Failed to begin packet!\n");
                }
                if (create_config_pb(encoded_message, &message_length, local_ip, local_port)) {
                    print("Sending config message of length %d\n", message_length);
                    size_t written = udp.write(encoded_message, message_length);
                    if (written != message_length) {
                        print("Only wrote %d instead of %d bytes!\n", written, message_length);
                    }
                }
                if (!udp.endPacket()) {
                    print("Failed to end packet!\n");
                }
                
                int packet_size = udp.parsePacket();
                if (packet_size) {
                    print("%d bytes from %s:%d\n", packet_size, udp.remoteIP().toString().c_str(), udp.remotePort());

                    Message decoded_message = Message_init_zero;    // reset the message
                    if (parse_pb(&udp, packet_size, &decoded_message)) {
                        if (decoded_message.type == MessageType_ACK) {
                            print("Received ack!\n");
                            print("Transition: DISCOVERY ==> DATA\n");
                            state = DATA;
                        }
                    }
                }

                vTaskDelay(DISCOVERY_CONFIG_MS / portTICK_PERIOD_MS);

                break;
            }
            /*
                State: DATA

                Behavior: Listen for data messages and parse. If the sequence number is larger 
                than the last received, immediately send to LEDs and update the last seen
                sequence number.
                
                Transition: On receipt of discovery message, transition to DISCOVERY state.
            */
            case DATA: {
                int packet_size = udp.parsePacket();
                if (packet_size) {
                    print("%d bytes from %s:%d\n", packet_size, udp.remoteIP().toString().c_str(), udp.remotePort());

                    Message decoded_message = Message_init_zero;    // reset the message
                    if (parse_pb(&udp, packet_size, &decoded_message)) {
                        if (decoded_message.type == MessageType_DATA) {
                            print("Received data!\n");
                            print("Sequence number = %d\n", decoded_message.sequence_number);
                        }
                    }
                }
                break;
            }
            default: {
                print("Client state %d not recognized!\n", state);
                break;
            }
        }
    }
}

/*
    Creates a serialized config message, stores it in the buffer pointer and
    updates the message_length variable.
*/
bool create_config_pb(uint8_t *buffer, size_t *message_length, IPAddress local_ip, uint16_t local_port) {
    bool status;

    // Initialize the message memory
    Message message = Message_init_zero;

    // Create a stream that will write to our buffer
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, MAX_MESSAGE_BYTES);

    // Fill the message. IMPORTANT: the has_ fields must be explicitly set to
    // true, otherwise they will not be encoded.
    message.has_sender = true;
    message.sender = Sender_CLIENT_AMBILIGHT;

    message.has_type = true;
    message.type = MessageType_CONFIG;

    message.has_config = true;
    message.config.has_ipv4 = true;
    strlcpy(message.config.ipv4, local_ip.toString().c_str(), sizeof(message.config.ipv4) - 1);

    message.config.has_port = true;
    message.config.port = local_port;

    message.config.has_led_format = true;
    message.config.led_format = LedFormat_RECTANGULAR_PERIMETER;

    message.config.has_num_leds = true;
    message.config.num_leds = NUM_LEDS;

    // Encode the message
    status = pb_encode(&stream, Message_fields, &message);
    *message_length = stream.bytes_written;

    if (!status) {
        print("Encoding failed: %s\n", PB_GET_ERROR(&stream));
    }

    return status;
}

/*
    Parses a protobuf packet received via UDP and stores the contents in memory
    at the passed in message pointer. Returns true is successful, false
    otherwise.
*/
bool parse_pb(WiFiUDP *udp, int packet_size, Message *message) {
    bool status;
    uint8_t buffer[MAX_MESSAGE_BYTES];
    int len = udp->read(buffer, MAX_MESSAGE_BYTES - 1);

    // Null-terminate
    if (len > 0) {
        buffer[len] = 0;
    }

    // Create a stream that reads from the buffer
    pb_istream_t stream = pb_istream_from_buffer(buffer, packet_size);

    // Now decode the message
    status = pb_decode(&stream, Message_fields, message);

    if (!status) {
        printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
    }

    return status;
}