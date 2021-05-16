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


// ---------- LED ----------
#define BLINK_ON_PERIOD 500
#define BLINK_OFF_PERIOD 1000

#define LED_IO_V07 14
#define LED_IO_V10 4
static uint8_t led_io;


void led_blinking_process() {
    static uint8_t state = 0;
    static uint32_t next = 0;

    if (millis() > next) {
        switch (state) {
        case 0:
            digitalWrite(led_io, HIGH);
            next = millis() + BLINK_ON_PERIOD;
            state = 1;
            break;

        case 1:
            digitalWrite(led_io, LOW);
            next = millis() + BLINK_OFF_PERIOD;
            state = 0;
            break;
        }
    }
}

void led_setup() {
    pinMode(led_io, OUTPUT);
    digitalWrite(led_io, LOW);
}


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
String  rssi     = "RSSI --";
String  snr      = "SNR --";
String  packSize = "--";
String  packet   = "";


void oled_setup() {
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    delay(100);
}


// ---------- Virtual TUBE ----------
// screen /dev/ttyUSB1 38400,cs8,parenb,-parodd
#define SERIAL_V Serial2
#define VTUBE_UART_BAUDRATE  38400
#define VTUBE_UART_CONFIG    SERIAL_8E1
#define VTUBE_TX 2
#define VTUBE_RX 13


void vtube_setup() {
    SERIAL_V.begin(VTUBE_UART_BAUDRATE, VTUBE_UART_CONFIG, VTUBE_RX, VTUBE_TX);
    while (!SERIAL_V);
    while (SERIAL_V.available()) {
        SERIAL_V.read();
    }
}

void vtube_forwarding_process() {
    if (SERIAL_V.available()) {
        String input = SERIAL_V.readStringUntil('\n');

        // TODO: Pass 'input' through LoRa network to node id 0
        Serial.print("[VTUBE] >> ");
        Serial.println(input);
    }
}


// ---------- Bluetooth-serial ----------
#define BT_NAME "ESP32-LoRa-Relay"
BluetoothSerial bt;


void bt_setup() {
    bt.begin(BT_NAME);
}


// ---------- LoRa ----------
// #define BAND    868E6
#define BAND    923E6

#define LORA_SCK  5   // GPIO5  -- SX1278's SCK
#define LORA_MISO 19  // GPIO19 -- SX1278's MISO
#define LORA_MOSI 27  // GPIO27 -- SX1278's MOSI
#define LORA_SS   18  // GPIO18 -- SX1278's CS
#define LORA_DI0  26  // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define LORA_RST  23  // GPIO23 -- SX1278's RESET


void on_flood_receive(void *message, uint8_t len) {
    debug("Received: %s", message);
}

void lora_setup() {
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
    if (!LoRa.begin(BAND)) {
        Serial.println("[DEBUG] Starting LoRa failed!");
        while (1);
    }

    LoRa.setSpreadingFactor(12);  // ranges from 6-12, default 7 see API docs. Changed for ver 0.1 Glacierjay
    // LoRa.setSignalBandwidth(7.8E3);  // signalBandwidth - signal bandwidth in Hz, defaults to 125E3.
                                     // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3.
    // LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);  // Set maximum Tx power to 20 dBm (17 is default).
                                                  // https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md#tx-power
    // LoRa.setGain(0);  // Supported values are between 0 and 6. If gain is 0, AGC will be enabled and LNA gain will not be used.
                      // Else if gain is from 1 to 6, AGC will be disabled and LNA gain will be used.

    flood_init();
    flood_set_rx_handler(on_flood_receive);

    Serial.println("[DEBUG] Starting LoRa ok");
}

void cbk(int packetSize) {  // XXX: leave it here for reference.
    packet   = "";
    packSize = String(packetSize, DEC);
    for (int i = 0; i < packetSize; i++) {
        packet += (char)LoRa.read();
    }

    rssi = "RSSI " + String(LoRa.packetRssi(), DEC);
    snr  = "SNR "  + String(LoRa.packetSnr(), 2);

    // lora_data();
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);

    display.drawString(0, 0, rssi + ", " + snr);
    display.drawString(0, 15, "Recv: " + packSize + " bytes");
    display.drawStringMaxWidth(0, 26, 128, packet);

    display.display();

    // String str = gps_datetime + ", " + gps_loc + ", " + rssi + ", " + snr + ", " + packet;
    // Serial.println("[DEBUG] " + str);
    // if (bt.connected()) {
    //     bt.println(str);
    // }
}

void test_routing_send_to_zero() {
    if (getAddress() == 0)
        return;

    static uint32_t next = millis() + 10000 + ((rand() & 0x03) << 10);
    if (millis() > next) {
        flood_send_to(0, "Hello", 6);  // XXX: tx for testing
        next = millis() + 10000 + ((rand() & 0x03) << 10);
    }
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

        led_io = LED_IO_V07;
    } else {
        Serial.println("[DEBUG] Starting AXP192 succeeded! -- guessing, its version >= V1.0");
        is_tbeam_version_less_v1 = false;

        axp_setup();
        led_io = LED_IO_V10;
    }

    oled_setup();   // OLED
    led_setup();    // LED
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
