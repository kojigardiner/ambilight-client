#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>

#include "CLI.h"
#include "CLIMenu.h"
#include "../Constants.h"
#include "../Utils/Utils.h"

void start_cli() {
    CLIMenu menu_main, menu_wifi, menu_clear_prefs;

    // Main
    const CLIMenuItem items_main[] = {
        CLIMenuItem("Setup wifi", &menu_wifi),
        CLIMenuItem("Clear preferences", &menu_clear_prefs),
    };
    menu_main = CLIMenu("Preferences Menu", items_main, ARRAY_SIZE(items_main));

    // Wifi
    const CLIMenuItem items_wifi[] = {
        CLIMenuItem("Check current wifi SSID", &check_wifi_prefs),
        CLIMenuItem("Setup new wifi SSID (scan)", &set_wifi_prefs_scan),
        CLIMenuItem("Setup new wifi SSID (manual)", &set_wifi_prefs),
        CLIMenuItem("Test wifi connection", &test_wifi_connection),
    };
    menu_wifi = CLIMenu("Wifi Menu", items_wifi, ARRAY_SIZE(items_wifi), &menu_main);

    // Clear prefs
    const CLIMenuItem items_clear[] = {
        CLIMenuItem("Yes", &clear_prefs),
    };
    menu_clear_prefs = CLIMenu("Are you sure you want to clear all preferences?", items_clear, ARRAY_SIZE(items_clear), &menu_main);

    menu_main.run_cli();
}

void check_wifi_prefs() {
    Preferences prefs;
    prefs.begin(APP_NAME, false);

    char wifi_ssid[CLI_MAX_CHARS];

    print("Current wifi network: ");
    if (!prefs.getString(PREFS_WIFI_SSID_KEY, wifi_ssid, CLI_MAX_CHARS)) {
        print("*** not set ***\n");
    } else {
        print("%s\n", wifi_ssid);
    }
    prefs.end();
}

void set_wifi_prefs() {
    Preferences prefs;
    prefs.begin(APP_NAME, false);
    set_pref(&prefs, PREFS_WIFI_SSID_KEY);
    set_pref(&prefs, PREFS_WIFI_PASS_KEY);
    prefs.end();
}

void set_wifi_prefs_scan() {
    print("\nScanning wifi networks...\n");
    int n = WiFi.scanNetworks();
    if (n == 0) {
        print("No networks found, try using manual method to set wifi SSID.\n");
    } else {
        print("%d networks found:\n", n);
        for (int i = 0; i < n; i++) {
            print("\t%d. %s (%d)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }
    }
    print("\t0. Return\n");

    char rcvd[CLI_MAX_CHARS];
    int idx = -1;
    while (idx < 0 || idx > n) {
        print("Select a network: ");
        get_input(rcvd);
        idx = atoi(rcvd);
        if (idx == 0) return;
    }
    print("%s selected.\nEnter pass: ", WiFi.SSID(idx - 1).c_str());
    get_input(rcvd);

    Preferences prefs;
    prefs.begin(APP_NAME, false);

    set_pref(&prefs, PREFS_WIFI_SSID_KEY, WiFi.SSID(idx - 1).c_str());
    set_pref(&prefs, PREFS_WIFI_PASS_KEY, rcvd);
    prefs.end();
}

void clear_prefs() {
    Preferences prefs;
    prefs.begin(APP_NAME, false);
    print("Clearing preferences\n");
    prefs.clear();
    prefs.end();
}

void test_wifi_connection() {
    print("\n");
    connect_wifi();
    print("disconnecting\n");
    WiFi.disconnect();
}