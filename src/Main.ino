/**
 * 
 * Serial as DEBUG & CLI
 * Serial1 as GPS
 * Serial2 as VTube connected with RTU
 * BT as data log
 * 
 */
#include "images.h"  // Note on a bug: arduino-vscode cannot find the header
                     //   named with alphabet comming before the main .ino file.
#include <strings.h>

#include <axp20x.h>
#include <SSD1306.h>
#include <BluetoothSerial.h>

#include <SPI.h>
#include <LoRa.h>

#include <TinyGPS++.h>
#include <SimpleCLI.h>

#include "all_headers.h"
#include "flood.h"

// ---------- Setup ----------
void setup() {
    config_setup(); // Load configuration
    cli_setup();    // CLI
    oled_setup();   // OLED

    bool is_tbeam_version_less_v1 = axp_setup();  // Init axp20x and return T-Beam Version

    led_setup(is_tbeam_version_less_v1);  // LED
    lora_setup();   // LoRa
    gps_setup(is_tbeam_version_less_v1);  // GPS

    vtube_setup();  // Virtual Tube connected to weather station


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


    // ----------------
    // XXX: For testing only
    // ----------------
    // test_routing_send_to_zero();
}
