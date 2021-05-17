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
AXP20X_Class axp;
#define AXP_SDA 21
#define AXP_SCL 22
#define AXP_IRQ 35


bool axp_setup() {
    bool is_tbeam_version_less_v1;

    Wire.begin(AXP_SDA, AXP_SCL);
    if (axp.begin(Wire, AXP192_SLAVE_ADDRESS) == AXP_FAIL) {
        Serial.println("[DEBUG] Starting AXP192 failed! -- guessing, this is the V0.7");
        is_tbeam_version_less_v1 = true;
    } else {
        Serial.println("[DEBUG] Starting AXP192 succeeded! -- guessing, its version >= V1.0");

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


// ---------- OLED ----------
SSD1306 display(0x3C, 21, 22);


void oled_setup() {
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    delay(100);

    // Using...
    // display.clear();
    // display.setTextAlignment(TEXT_ALIGN_LEFT);
    // display.setFont(ArialMT_Plain_10);
    // display.drawString(0, 0, rssi + ", " + snr);
    // display.drawString(0, 15, "Recv: " + packSize + " bytes");
    // display.drawStringMaxWidth(0, 26, 128, packet);
    // display.display();
}


// ---------- Bluetooth-serial ----------
#define BT_NAME "ESP32-LoRa-Relay"
BluetoothSerial bt;


void bt_setup() {
    bt.begin(BT_NAME);

    // Using...
    // if (bt.connected()) {
    //     bt.println(str);
    // }
}


// ---------- Setup ----------
void setup() {
    Serial.begin(115200);
    while (!Serial);

    bool is_tbeam_version_less_v1 = axp_setup();  // Init axp20x and return T-Beam Version

    oled_setup();   // OLED
    led_setup(is_tbeam_version_less_v1);  // LED
    lora_setup();   // LoRa
    bt_setup();     // Bluetooth-Serial
    gps_setup(is_tbeam_version_less_v1);  // GPS
    vtube_setup();  // Virtual Tube connected to weather station
    cli_setup();    // CLI


    // ----------------
    // XXX: For testing only
    // ----------------
    // while (true)
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
    test_routing_send_to_zero();  //  XXX: for testing only
}
