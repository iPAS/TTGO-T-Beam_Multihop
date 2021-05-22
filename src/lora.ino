#include <strings.h>

#include <SSD1306.h>

#include <SPI.h>
#include <LoRa.h>

#include "all_headers.h"
#include "flood.h"


// Computed with https://www.thethingsnetwork.org/airtime-calculator
// Airtime should be about 513ms on sending of 100-byte payload
#define LORA_BAND   923E6   // 868E6
#define LORA_SF     11      // SF 6-12, default 7
#define LORA_BW     500E3   // BW in Hz, defaults 125E3.
                            // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3.
#define LORA_TX_POWER 20    // Set maximum Tx power to 20 dBm (17 is default). See https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md#tx-power

#define LORA_SCK  5   // GPIO5  -- SX1278's SCK
#define LORA_MISO 19  // GPIO19 -- SX1278's MISO
#define LORA_MOSI 27  // GPIO27 -- SX1278's MOSI
#define LORA_SS   18  // GPIO18 -- SX1278's CS
#define LORA_DI0  26  // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define LORA_RST  23  // GPIO23 -- SX1278's RESET


// ----------------------------------------------------------------------------
void on_flood_receive(void *message, uint8_t len) {
    Serial.print("Received <");
    Serial.print(len);
    Serial.print("> ");
    Serial.println((char *)message);

    // TODO: print out to Serial to tell that who sent
}


// ----------------------------------------------------------------------------
void lora_setup() {
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
    if (!LoRa.begin(LORA_BAND)) {
        Serial.println("[DEBUG] Starting LoRa failed!");
        while (1);
    }

    LoRa.setSpreadingFactor(LORA_SF);
    LoRa.setSignalBandwidth(LORA_BW);
    LoRa.setTxPower(LORA_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);
    // LoRa.setGain(0);    // Supported values are between 0 and 6. If gain is 0, AGC will be enabled and LNA gain will not be used.
                        // Else if gain is from 1 to 6, AGC will be disabled and LNA gain will be used.

    flood_init();
    flood_set_rx_handler(on_flood_receive);

    Serial.println("[DEBUG] Starting LoRa ok");
}


void test_routing_send_to_zero() {
    if (getAddress() == SINK_ADDRESS)
        return;

    static uint32_t next = millis() + 10000 + ((rand() & 0b011) << 10);
    if (millis() > next) {
        flood_send_to(0, "Hello", 6);
        next = millis() + 10000 + ((rand() & 0b011) << 10);
    }
}
