#pragma once

// This header and its associated CLI.cpp file define functions that make use of the 
// CLIMenu and CLIMenuItem classes to create a command-line interface for setting up 
// the Audiobox. Many of the functions read/write from the ESP32's non-volatile
// Preferences memory. The CLI requires a USB connection to the ESP32.

// Sets up the CLIMenu and CLIMenuItem objects and starts the CLI loop.
void start_cli();

// Prints the state of the wifi SSID preferences.
void check_wifi_prefs();

// Asks the user to enter wifi SSID/password and saves them to preferences.
void set_wifi_prefs();

// Scans existing wifi networks and asks the user to select an SSID and enter its
// password. Then saves the results to preferences.
void set_wifi_prefs_scan();

// Clears all preferences.
void clear_prefs();

// Attempts to connect to the currently set wifi SSID and prints the results.
void test_wifi_connection();
