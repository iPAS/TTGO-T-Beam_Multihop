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

#include <SSD1306.h>
#include <axp20x.h>
#include <BluetoothSerial.h>

#include <SPI.h>
#include <LoRa.h>

#include <TinyGPS++.h>
#include <SimpleCLI.h>

#include "all_headers.h"
#include "flood.h"


// ---------- AXP192 ----------
AXP20X_Class axp;
#define AXP_SDA 21
#define AXP_SCL 22
#define AXP_IRQ 35


void axp_setup() {
    axp.setLDO2Voltage(3300);   // LoRa VDD
    axp.setLDO3Voltage(3300);   // GPS  VDD
    axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);  // LORA radio
    axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);  // GPS main power

    axp.setDCDC1Voltage(3300);  // for the OLED power
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);

    axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);

    // axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
}


// ---------- OLED ----------
SSD1306 display(0x3C, 21, 22);


void oled_setup() {
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    delay(100);
}


// ---------- Bluetooth-serial ----------
#define BT_NAME "ESP32-LoRa-Relay"
BluetoothSerial bt;


void bt_setup() {
    bt.begin(BT_NAME);
}


// ---------- Setup ----------
void setup() {
    bool is_tbeam_version_less_v1;

    Serial.begin(115200);
    while (!Serial);

    // Version validation
    Wire.begin(AXP_SDA, AXP_SCL);
    if (axp.begin(Wire, AXP192_SLAVE_ADDRESS) == AXP_FAIL) {
        Serial.println("[DEBUG] Starting AXP192 failed! -- guessing, this is the V0.7");
        is_tbeam_version_less_v1 = true;
    } else {
        Serial.println("[DEBUG] Starting AXP192 succeeded! -- guessing, its version >= V1.0");
        is_tbeam_version_less_v1 = false;
        axp_setup();
    }

    oled_setup();   // OLED
    led_setup(is_tbeam_version_less_v1);  // LED
    lora_setup();   // LoRa
    bt_setup();     // Bluetooth-Serial
    gps_setup(is_tbeam_version_less_v1);  // GPS
    vtube_setup();   // Virtual Tube connected to weather station
    cli_setup();    // CLI


    // ----------------
    // For testing only
    // ----------------
    // test_ztimer();  // XXX: for testing only
}


// ---------- Main ----------
void loop() {
    // LED blinking
    led_blinking_process();

    // Process GPS data
    gps_decoding_process();


    // Process LoRa received packet
    // XXX: use calling-back function instead
    // int packetSize = LoRa.parsePacket();
    // if (packetSize) {
    //     cbk(packetSize);
    // }

    // ----------------
    // For testing only
    // ----------------
    test_routing_send_to_zero();  //  XXX: for testing only


    // Forward Data received from Virtual Tube
    vtube_forwarding_process();

    // Process command-line input
    cli_interpreting_process();
}
