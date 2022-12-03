#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <FastLED.h>

#include "Constants.h"
#include "CLI/CLI.h"
#include "Timer/Timer.h"
#include "Utils/Utils.h"

// Nanopb
#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "proto/ambilight.pb.h"

// Function Prototypes
bool parse_pb(WiFiUDP *udp, int packet_size, Message *message);
bool create_pb(MessageType type, uint8_t *buffer, size_t *message_length);
bool create_pb(MessageType type, uint8_t *buffer, size_t *message_length, IPAddress local_ip, uint16_t local_port);

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

    Timer timer = Timer();
    Timer timer_ack = Timer();

    state_t state = START;


    for (;;) {
        switch (state) {
            /*
                State: START
                Behavior: Setup UDP port to receive broadcast messages. Set wifi 
                power saving.
                Transition: Once setup, transition to IDLE.
             */
            case START: {
                udp.stop();
                local_ip = WiFi.localIP();
                local_port = UDP_BROADCAST_PORT;
                print("Starting UDP at %s:%d\n", local_ip.toString().c_str(), local_port);
                udp.begin(local_port);
                print("Enabling wifi power saving\n");
                WiFi.setSleep(true);
                print("Transition: START ==> IDLE\n");
                state = IDLE;
                break;
            }
            /*
                State: IDLE
                Behavior: Listen for discovery message.
                
                Transition: On receipt of discovery message, store server's IP/port, 
                start transition to TO_DISCOVERY state.
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
                            
                            // Comment out the code below to keep using the UDP_BROADCAST_PORT 
                            // instead of randomizing it.
                            // In a real-world use case we probably want to randomize the port to
                            // prevent server spoofing attacks. Continuing to use UDP_BROADCAST_PORT
                            // allows us to keep listening for discovery packets 
                            // once we start collecting data.

                            udp.stop();
                            local_port = (rand() % (MAX_UDP_PORT - MIN_UDP_PORT)) + MIN_UDP_PORT;
                            print("Restarting UDP service on port %d\n", local_port);
                            udp.begin(local_port);

                            print("Transition: IDLE ==> TO_DISCOVERY\n");
                            state = TO_DISCOVERY;
                        }
                    }
                }
                break;
            }
            /*
                State: TO_DISCOVERY

                Behavior: Set discovery timeout timer.

                Transition: Immediately transition to DISCOVERY.
            */
            case TO_DISCOVERY: {
                timer.set_timeout_ms(DISCOVERY_TIMEOUT_MS);
                print("Transition: TO_DISCOVERY ==> DISCOVERY\n");
                state = DISCOVERY;
                break;
            }
            /*
                State: DISCOVERY

                Behavior: Send config packets every DISCOVERY_CONFIG_MS.

                Transition: On receipt of ACK, transition to TO_DATA state. If no ACK is
                received before timeout, transition to START state.
            */
            case DISCOVERY: {
                // Send config packet to server
                if (!udp.beginPacket(server_ip, server_port)) {
                    print("Failed to begin packet!\n");
                }
                if (create_pb(MessageType_CONFIG, encoded_message, &message_length, local_ip, local_port)) {
                    print("Sending config message of length %d\n", message_length);
                    size_t written = udp.write(encoded_message, message_length);
                    if (written != message_length) {
                        print("Only wrote %d instead of %d bytes!\n", written, message_length);
                    }
                }
                if (!udp.endPacket()) {
                    print("Failed to end packet!\n");
                }
                
                // Check for ack packet from server
                int packet_size = udp.parsePacket();
                if (packet_size) {
                    print("%d bytes from %s:%d\n", packet_size, udp.remoteIP().toString().c_str(), udp.remotePort());

                    Message decoded_message = Message_init_zero;    // reset the message
                    if (parse_pb(&udp, packet_size, &decoded_message)) {
                        if (decoded_message.type == MessageType_ACK_DISCOVERY) {
                            print("Received discovery ack!\n");
                            print("Transition: DISCOVERY ==> TO_DATA\n");
                            state = TO_DATA;
                        }
                    }
                }

                if (timer.has_elapsed()) {
                    print("Timed out\n");
                    print("Transition: DISCOVERY ==> START\n");
                    state = START;
                }
                vTaskDelay(DISCOVERY_CONFIG_MS / portTICK_PERIOD_MS);

                break;
            }
            /*
                State: TO_DATA

                Behavior: Set wifi power saving off. Set heartbeat timers.
                
                Transition: Immediately transition to DATA.
            */
            case TO_DATA: {
                WiFi.setSleep(false);
                timer.set_timeout_ms(HEARTBEAT_MS);
                timer_ack.set_timeout_ms(HEARTBEAT_ACK_MS);
                print("Disabling wifi power saving\n");
                print("Transition: TO_DATA ==> DATA\n");
                state = DATA;
                break;
            }
            /*
                State: DATA

                Behavior: Listen for data messages and parse. If the sequence number is larger 
                than the last received, immediately send to LEDs and update the last seen
                sequence number.
                
                Transition: On receipt of discovery message, transition back to IDLE state in
                order to re-establish connection.
            */
            case DATA: {
                // Check for data from server
                int packet_size = udp.parsePacket();
                if (packet_size) {
                    print("%d bytes from %s:%d\n", packet_size, udp.remoteIP().toString().c_str(), udp.remotePort());

                    Message decoded_message = Message_init_zero;    // reset the message
                    if (parse_pb(&udp, packet_size, &decoded_message)) {
                        if (decoded_message.type == MessageType_DATA) {
                            print("Received data!\n");
                            print("Sequence number = %d\n", decoded_message.sequence_number);
                            size_t to_copy = NUM_LEDS * 3;
                            if (decoded_message.data.led_data.size < to_copy) {
                                to_copy = decoded_message.data.led_data.size;
                            }
                            memcpy(leds, decoded_message.data.led_data.bytes, to_copy);
                            FastLED.show();
                        }
                        if (decoded_message.type == MessageType_ACK_HEARTBEAT) {
                            print("Received heartbeat ack!\n");
                            timer_ack.reset();
                        }
                        // if (decoded_message.type == MessageType_DISCOVERY) {
                        //     print("Received unexpected discovery message! Resetting\n");
                        //     print("Transition: DATA ==> IDLE\n");
                        //     state = IDLE;
                        // }
                    }
                }
                // Send heartbeat
                if (timer.has_elapsed(true)) {
                    if (!udp.beginPacket(server_ip, server_port)) {
                        print("Failed to begin packet!\n");
                    }
                    create_pb(MessageType_HEARTBEAT, encoded_message, &message_length);
                    print("Sending heartbeat message of length %d\n", message_length);
                    size_t written = udp.write(encoded_message, message_length);
                    if (written != message_length) {
                        print("Only wrote %d instead of %d bytes!\n", written, message_length);
                    }
                    if (!udp.endPacket()) {
                        print("Failed to end packet!\n");
                    }
                }
                // Check that we have received an ack from the server
                if (timer_ack.has_elapsed()) {
                    print("Timed out waiting for heartbeat ack from server!\n");
                    print("Transition: DATA ==> START\n");
                    state = START;
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
    Creates a serialized message, stores it in the buffer pointer and updates 
    the message_length variable.
*/
bool create_pb(MessageType type, uint8_t *buffer, size_t *message_length) {
    return create_pb(type, buffer, message_length, IPAddress(), 0);
}

/*
    Creates a serialized message, stores it in the buffer pointer and updates 
    the message_length variable.
*/
bool create_pb(MessageType type, uint8_t *buffer, size_t *message_length, IPAddress local_ip, uint16_t local_port) {
    bool status;

    // Initialize the message memory
    Message message = Message_init_zero;

    // Create a stream that will write to our buffer
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, MAX_MESSAGE_BYTES);

    // Fill the message. IMPORTANT: the has_ fields must be explicitly set to
    // true, otherwise they will not be encoded.
    message.has_sender = true;
    message.sender = Sender_CLIENT_AMBILIGHT;

    switch (type) {
        case MessageType_CONFIG: {
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
            break;
        }
        case MessageType_HEARTBEAT: {
            message.has_type = true;
            message.type = MessageType_HEARTBEAT;
            break;
        }
        default: {
            print("pb type not recognized!\n");
            return false;
        }
    }
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