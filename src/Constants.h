#pragma once

#include <Arduino.h>


// Strings
const char* const APP_NAME = "Ambilight";
const char* const MDNS_DOMAIN_NAME = "ambilight-client";

// Keys for Preferences dictionary
const char* const PREFS_WIFI_SSID_KEY = "wifi_ssid";
const char* const PREFS_WIFI_PASS_KEY = "wifi_pass";

// Networking
const uint8_t WIFI_LOCAL_IP[4] = {192, 168, 3, 179};
const uint8_t WIFI_GATEWAY[4] = {192, 168, 1, 1};
const uint8_t WIFI_SUBNET[4] = {255, 255, 0, 0};
const uint8_t WIFI_DNS1[4] = {8, 8, 8, 8};
const uint8_t WIFI_DNS2[4] = {8, 8, 4, 4};

// Macros
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define TIME_THIS(A)                       \
{                                          \
    unsigned long last_micros = micros();  \
    A;                                     \
    print("%d\n", micros() - last_micros); \
}

#define PIN_LED_STATUS 2    // Status LED on ESP32 Hiletgo board

// Maximum string lengths
#define HTTP_MAX_CHARS 512    // max number of chars for http processing
#define CLI_MAX_CHARS 256     // max number of chars for CLI input
#define MAX_CLI_MENU_ITEMS 9  // max items per level of CLI
#define MAX_CLI_MENU_TEXT 64  // max text length for CLI menu items

#define WIFI_TIMEOUT_MS 10000               // how long to wait on wifi connect before bailing out