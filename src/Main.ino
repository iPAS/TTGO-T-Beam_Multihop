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


// ---------- AXP192 ----------
#define AXP_SDA 21
#define AXP_SCL 22
#define AXP_IRQ 35

static AXP20X_Class axp;


bool axp_setup() {
    bool is_tbeam_version_less_v1;

    Wire.begin(AXP_SDA, AXP_SCL);
    if (axp.begin(Wire, AXP192_SLAVE_ADDRESS) == AXP_FAIL) {
        term_println("[DEBUG] Starting AXP192 failed! -- guessing, this is the V0.7");
        is_tbeam_version_less_v1 = true;
    } else {
        term_println("[DEBUG] Starting AXP192 succeeded! -- guessing, its version >= V1.0");

        axp.setLDO2Voltage(3300);   // LoRa VDD
        axp.setLDO3Voltage(3300);   // GPS  VDD
        axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);  // LORA radio
        axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);  // GPS main power

        axp.setDCDC1Voltage(3300);  // for the OLED power
        axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);

        axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
        axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);

        // axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);

        is_tbeam_version_less_v1 = false;
    }

    return is_tbeam_version_less_v1;
}


// ---------- Setup ----------
void setup() {
    cli_setup();    // CLI

    config_setup();  // Load configuration
    bt_setup();  // Bluetooth-Serial
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

    #ifndef LORA_CALLBACK_MODE
    lora_parsing_process();  // Parsing LoRa received packet
    #endif

    cli_interpreting_process();  // Process command-line input
    bt_cli_interpreting_process();  // Process command-line input through Bluetooth-serial


    // ----------------
    // XXX: For testing only
    // ----------------
    // test_routing_send_to_zero();
}
