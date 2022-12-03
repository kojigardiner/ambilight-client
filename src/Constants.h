#pragma once

#include <Arduino.h>

// Strings
const char* const APP_NAME = "Ambilight";
const char* const MDNS_DOMAIN_NAME = "ambilight-client";

// Keys for Preferences dictionary
const char* const PREFS_WIFI_SSID_KEY = "wifi_ssid";
const char* const PREFS_WIFI_PASS_KEY = "wifi_pass";

// State machine
typedef enum state {
    START,
    IDLE,
    TO_DISCOVERY,
    DISCOVERY,
    TO_DATA,
    DATA,
} state_t;

// Macros
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define TIME_THIS(A)                       \
{                                          \
    unsigned long last_micros = micros();  \
    A;                                     \
    print("%d\n", micros() - last_micros); \
}

#define PIN_LED_STATUS 2    // Status LED on ESP32 Hiletgo board
#define PIN_LED_CONTROL 12  // LED control line

// Maximum string lengths
#define HTTP_MAX_CHARS 512    // max number of chars for http processing
#define CLI_MAX_CHARS 256     // max number of chars for CLI input
#define MAX_CLI_MENU_ITEMS 9  // max items per level of CLI
#define MAX_CLI_MENU_TEXT 64  // max text length for CLI menu items

// Max protobuf message size
#define MAX_MESSAGE_BYTES 1460

// Timeouts
#define WIFI_TIMEOUT_MS 10000               // how long to wait on wifi connect before bailing out
#define DISCOVERY_CONFIG_MS 1000            // how long to wait between discovery loops
#define DISCOVERY_TIMEOUT_MS 10000          // how long to wait in discovery state before returning to idle
#define HEARTBEAT_MS 2000                   // how often to send a heartbeat to the server
#define HEARTBEAT_ACK_MS 10000              // how long wait for a heartbeat response

// UDP Port Range
#define UDP_BROADCAST_PORT 3000
#define MIN_UDP_PORT 49152
#define MAX_UDP_PORT 65535

// LED
#define NUM_LEDS 114