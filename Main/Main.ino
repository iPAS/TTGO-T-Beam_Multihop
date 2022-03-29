/**
 * @file Main.ino
 * @brief LoRa Relay Implementation on T-Beam ESP32 Platform with Motelib Flood Routing Support
 *
 * @author Pasakorn Tiwatthanont
 *
 * Serial as DEBUG & CLI
 * Serial1 as GPS
 * Serial2 as VTube connected with RTU
 * BT as data log
 *
 */
#include "all_headers.h"
#include "flood.h"

#include "images.h"  // Note on a bug: arduino-vscode cannot find the header
                     //   named with alphabet comming before the main .ino file.
#include <strings.h>

#include <axp20x.h>
#include <SSD1306.h>

#include <SPI.h>
#include <LoRa.h>

#include <TinyGPS++.h>
#include <SimpleCLI.h>


static bool do_axp_exist;  // T-Beam v0.7, early version, does't has AXP192 installed.

// ---------- Setup ----------
void setup() {
    config_setup(); // Load configuration
    cli_setup();    // CLI
    oled_setup();   // OLED

    do_axp_exist = axp_setup();  // Init axp20x and return T-Beam Version

    led_setup(do_axp_exist);    // LED
    lora_setup();               // LoRa
    gps_setup(do_axp_exist);    // GPS

    vtube_setup();              // Virtual Tube connected to weather station

    ota_setup();  // OTA

    // ----------------
    // XXX: For testing only
    // ----------------
    // test_ztimer();
    // test_vtube_loopback();
}

// ---------- Main ----------
void loop() {
    led_blinking_process();  // LED blinking
    gps_decoding_process();  // Process GPS data

    vtube_forwarding_process();  // Forward Data received from Virtual Tube

    cli_interpreting_process();  // Process command-line input

    lora_reporting_process();  // Report node status to #0

    if (do_axp_exist)
        axp_logging_process();  // Report energy usage on the node.

    ota_process();

    // ----------------
    // XXX: For testing only
    // ----------------
    // test_routing_send_to_zero();
}
