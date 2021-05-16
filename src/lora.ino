#include <strings.h>

#include <SSD1306.h>

#include <SPI.h>
#include <LoRa.h>

#include "all_headers.h"
#include "flood.h"


// #define BAND    868E6
#define BAND    923E6

#define LORA_SCK  5   // GPIO5  -- SX1278's SCK
#define LORA_MISO 19  // GPIO19 -- SX1278's MISO
#define LORA_MOSI 27  // GPIO27 -- SX1278's MOSI
#define LORA_SS   18  // GPIO18 -- SX1278's CS
#define LORA_DI0  26  // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define LORA_RST  23  // GPIO23 -- SX1278's RESET

String  rssi     = "RSSI --";
String  snr      = "SNR --";
String  packSize = "--";
String  packet   = "";


// ----------------------------------------------------------------------------
void on_flood_receive(void *message, uint8_t len) {
    debug("Received: %s", message);
}


// ----------------------------------------------------------------------------
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
