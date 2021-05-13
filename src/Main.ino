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
#include <SPI.h>
#include <LoRa.h>
#include <SSD1306.h>
#include <TinyGPS++.h>
#include <axp20x.h>

#include <BluetoothSerial.h>
#include <SimpleCLI.h>

#include "all_headers.h"
#include "flood.h"


// ---------- LED ----------
#define LED_IO_V07 14
#define LED_IO_V10 4
static uint8_t led_io;


void led_toggle_process() {
    static uint8_t state = 0;
    static uint32_t start = millis();

    switch (state) {
        case 0:
            if (millis() - start > 500) {
                digitalWrite(led_io, HIGH);
                start = millis();
                state = 1;
            }
            break;
        case 1:
            if (millis() - start > 500) {
                digitalWrite(led_io, LOW);
                start = millis();
                state = 0;
            }
            break;
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


// ---------- GPS ----------
#define GPS_BAUDRATE 9600

static uint8_t gps_tx;
static uint8_t gps_rx;

#define GPS_TX_V07 15
#define GPS_RX_V07 12

#define GPS_TX_V10 12
#define GPS_RX_V10 34

TinyGPSPlus gps;
String gps_datetime = "@ --";
String gps_loc  = "SAT --, LAT --, LON --, ALT --";


void gps_setup() {
    Serial1.begin(GPS_BAUDRATE, SERIAL_8N1, gps_rx, gps_tx);
    while (!Serial1);
    while (Serial1.available()) {
        Serial1.read();
    }
}


// ---------- Virtual Tube ----------
// screen /dev/ttyUSB1 38400,cs8,parenb,-parodd
#define TUBE_UART_BAUDRATE  38400
#define TUBE_UART_CONFIG    SERIAL_8E1
#define TUBE_TX 14
#define TUBE_RX 13


void tube_setup() {
    Serial2.begin(TUBE_UART_BAUDRATE, TUBE_UART_CONFIG, TUBE_RX, TUBE_TX);
    while (!Serial2);
    while (Serial2.available()) {
        Serial2.read();
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

    // LoRa.onReceive(cbk); // XXX: For testing if not call flood_init().
    // LoRa.receive();      // XXX: Receieve mode while in the main loop instead of callback function -- cbk.'

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

    String str = gps_datetime + ", " + gps_loc + ", " + rssi + ", " + snr + ", " + packet;
    // Serial.println("[DEBUG] " + str);
    if (bt.connected()) {
        bt.println(str);
    }
}

void send_to_zero() {
    if (getAddress == 0)
        return;

    static uint32_t start = millis();
    if (millis() - start > 2000)
    {
        flood_send_to(0, "Hello", 6);  // XXX: tx for testing
        start = millis();
    }
}


// ---------- CLI ----------
SimpleCLI cli;
Command cmd_hello;
Command cmd_node_id;


boolean isNumeric(String str) {  // http://tripsintech.com/arduino-isnumeric-function/
    unsigned int stringLength = str.length();

    if (stringLength == 0) {
        return false;
    }

    boolean seenDecimal = false;

    for(unsigned int i = 0; i < stringLength; ++i) {
        if (isDigit(str.charAt(i))) {
            continue;
        }

        if (str.charAt(i) == '.') {
            if (seenDecimal) {
                return false;
            }
            seenDecimal = true;
            continue;
        }
        return false;
    }
    return true;
}

void errorCallback(cmd_error *e) {
    CommandError cmdError(e); // Create wrapper object
    Serial.println("[CLI] " + cmdError.toString());
}

void on_cmd_hello(cmd *c) {
    Command cmd(c);
    Serial.println("[CLI] hello: Hello!");
}

void on_cmd_node_id(cmd *c) {
    Command cmd(c);
    Argument idArg = cmd.getArgument("id");
    long id = idArg.getValue().toInt();
    bool legal_id = false;

    if (idArg.isSet()) {  // The argument is provided.
        if (id == 0) {
            if (isNumeric(idArg.getValue()))
                legal_id = true;
        } else
        if (id > 0 && id < 65536) {
            legal_id = true;
        }

        if (legal_id) {
            // Set new id
            Serial.print("[CLI] node_id: new id ");
            Serial.println(setAddress(id));
        } else {
            // Illegal id
            Serial.println("[CLI] node_id: illegal id");
        }

    } else {  // No argument
        // Ask for current id
        Serial.print("[CLI] node_id: current id ");
        Serial.println(getAddress());
    }
}

void cli_setup() {
    cli.setOnError(errorCallback); // Set error Callback

    cmd_hello = cli.addCommand("hello", on_cmd_hello);

    cmd_node_id = cli.addCommand("node_id", on_cmd_node_id);
    cmd_node_id.addPositionalArgument("id", "");
}


// ---------- Setup ----------
void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Version validation
    Wire.begin(AXP_SDA, AXP_SCL);
    if (axp.begin(Wire, AXP192_SLAVE_ADDRESS) == AXP_FAIL) {
        Serial.println("[DEBUG] Starting AXP192 failed! -- guessing, this is the V0.7");

        led_io = LED_IO_V07;
        gps_tx = GPS_TX_V07;
        gps_rx = GPS_RX_V07;

    } else {
        Serial.println("[DEBUG] Starting AXP192 succeeded! -- guessing, its version >= V1.0");

        axp_setup();
        led_io = LED_IO_V10;
        gps_tx = GPS_TX_V10;
        gps_rx = GPS_RX_V10;
    }

    oled_setup();   // OLED
    led_setup();    // LED
    lora_setup();   // LoRa
    bt_setup();     // Bluetooth-Serial
    gps_setup();    // GPS
    tube_setup();   // Virtual Tube connected to weather station
    cli_setup();    // CLI


    // ----------------
    // For testing only
    // ----------------
    // zTimerTest();  // XXX: for testing only

}


// ---------- Main ----------
void loop() {
    // LED toggle
    led_toggle_process();

    // Process GPS data
    while (Serial1.available()) {
        gps.encode(Serial1.read());
    }

    if (gps.satellites.isValid() && gps.time.isUpdated() && gps.location.isValid()) {
        // Example: http://arduiniana.org/libraries/tinygpsplus/
        // "T --, SAT --, LAT --, LON --, ALT --, ";
        // gps_datetime = "@ "         + String(gps.date.day())  + ":" + String(gps.date.month())  + ":" + String(gps.date.year()) + " ";
        // gps_datetime = gps_datetime + String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
        char s_datetime[25];
        sprintf(s_datetime, "@ %02u-%02u-%04u %02u:%02u:%02u",
            gps.date.day(),  gps.date.month(),  gps.date.year(),
            gps.time.hour(), gps.time.minute(), gps.time.second());
        gps_datetime = String(s_datetime);

        gps_loc  = "SAT " + String(gps.satellites.value());
        gps_loc  = gps_loc + ", " + "LAT " + String(gps.location.lat(), 6);
        gps_loc  = gps_loc + ", " + "LON " + String(gps.location.lng(), 6);
        gps_loc  = gps_loc + ", " + "ALT " + String(gps.altitude.meters());
    }

    // Process LoRa received packet
    // XXX: use calling-back function instead
    // int packetSize = LoRa.parsePacket();
    // if (packetSize) {
    //     cbk(packetSize);
    // }


    // ----------------
    // For testing only
    // ----------------
    send_to_zero();  //  XXX: for testing only


    // Forward Data received from Virtual Tube
    if (Serial2.available()) {
        String input = Serial2.readStringUntil('\n');

        // TODO: Pass 'input' through LoRa network to node id 0
        Serial.print("[TUBE] >> ");
        Serial.println(input);
    }

    // Process command-line input
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');  // Read out string from the serial monitor
        cli.parse(input);  // Parse the user input into the CLI
    }

    if (cli.errored()) {
        CommandError cmdError = cli.getError();

        Serial.print("[CLI] Error: ");
        Serial.println(cmdError.toString());

        if (cmdError.hasCommand()) {
            Serial.print("[CLI] Did you mean \"");
            Serial.print(cmdError.getCommand().toString());
            Serial.println("\"?");
        }
    }
}
