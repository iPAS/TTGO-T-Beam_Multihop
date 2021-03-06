#include "all_headers.h"
#include "neighbor.h"
#include "flood.h"

#include <strings.h>

#include <SSD1306.h>
#include <SPI.h>
#include <LoRa.h>


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

#define LORA_REPORT_PERIOD_INIT 20000
#define LORA_REPORT_PERIOD      (60000*10)  // Ten minute

static uint32_t next_report_millis;

// ----------------------------------------------------------------------------
void on_neighbor_update(neighbor_t *nb) {
    RadioRxStatus status;
    radioGetRxStatus(&status);
    nb->rssi = status.rssi;
    nb->snr = status.snr;
}

// ----------------------------------------------------------------------------
bool report_status_to(Address sink) {
    char buf[100];  // Guess max payload as per LoRa packet. Max = sizeof(buf)-1
    char *p = buf;
    uint8_t i, cnt;
    neighbor_t *nb = neighbor_table();
    for (i = 0, cnt = 0; i < MAX_NEIGHBOR; i++, nb++) {
        if (nb->addr != BROADCAST_ADDR) {
            uint8_t len;

            if (cnt == 0) {
                len = snprintf(p, sizeof(buf)-cnt, ">STS\n");
                p += len;
                cnt += len;

                len = snprintf(p, sizeof(buf)-cnt, " !%s\n", __GIT_SHA1_ID__);
                p += len;
                cnt += len;
            }

            len = snprintf(p, sizeof(buf)-cnt, " @%d:%d,%.2f\n", nb->addr, nb->rssi, nb->snr);
            p += len;
            cnt += len;

            if (cnt >= sizeof(buf)-1) {
                sprintf(p-3, " ...");  // 'more' indicator
                break;
            }
        }
    }

    if (cnt == 0)
        return true;  // No any relation data

    term_printf("[LORA] @%d report:%d status to @%d:", getAddress(), cnt, sink);
    term_print(buf);
    term_println("[/LORA]");

    if (getAddress() == sink)
        return true;  // 'sink' needs none transaction.

    return flood_send_to(sink, buf, cnt);  // Not send NULL.
}

// ----------------------------------------------------------------------------
bool report_gps_to(Address sink) {
    char *str = gps_update_str(">GPS\n %s\n %s\n %s\n");
    if (str == NULL) {
        term_println("[LORA] report_gps_to(): gps_update_str() return NULL!");
        return false;
    }
    uint8_t cnt = strlen(str);

    term_printf("[LORA] @%d report:%d GPS to @%d:", getAddress(), cnt, sink);
    term_print(str);
    term_println("[/LORA]");

    return flood_send_to(SINK_ADDRESS, str, cnt);
}

// ----------------------------------------------------------------------------
bool report_axp_to(Address sink) {
    char *str = axp_update_str(">AXP\n %s\n %s\n %s\n");
    if (str == NULL) {
        term_println("[LORA] report_axp_to(): axp_update_str() return NULL!");
        return false;
    }
    uint8_t cnt = strlen(str);

    term_printf("[LORA] @%d report:%d AXP to @%d:", getAddress(), cnt, sink);
    term_print(str);
    term_println("[/LORA]");

    return flood_send_to(SINK_ADDRESS, str, cnt);
}

// ----------------------------------------------------------------------------
void on_flood_receive(void *message, uint8_t len) {
    RoutingHeader *hdr = (RoutingHeader*)message;
    uint8_t *data = &((uint8_t *)message)[sizeof(RoutingHeader)];
    uint8_t data_len = len - sizeof(RoutingHeader);

    term_printf("[D] @%d recv:%d frm @%d #%d ^%d >",
        hdr->finalSink, len, hdr->originSource, hdr->seqNo, hdr->hopCount);

    int16_t i;
    for (i = 0; i < data_len; i++) {
        term_print((char)data[i]);
    }
    term_println("[/D]");


    // Command processing
    for (i = 0; i < sizeof(remote_commands)/sizeof(remote_commands[0]); i++) {
        if (strncmp((const char *)data, remote_commands[i], data_len) == 0) {

            if (remote_commands[i] == REMOTE_CMD_HELLO) {
                // Do nothing
            }
            else

            if (remote_commands[i] == REMOTE_CMD_PING) {
                if (flood_send_to(hdr->originSource, REMOTE_RESP_PING, strlen(REMOTE_RESP_PING))) {
                    term_printf("[LORA] pong: back to %d", hdr->originSource);
                }
                else {
                    term_println("[LORA] pong: flood_send_to() failed!");
                }
            }
            else

            if (remote_commands[i] == REMOTE_CMD_RESET) {
                term_printf("[LORA] reset: from %d", hdr->originSource);
                ESP.restart();  // TODO: delay reset!!
            }

        }
    }
}

// ----------------------------------------------------------------------------
void lora_setup() {
    radio_setup();

    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
    if (!LoRa.begin(LORA_BAND)) {
        term_println("[LORA] Starting failed!");
        while (1);
    }

    LoRa.setSpreadingFactor(LORA_SF);
    LoRa.setSignalBandwidth(LORA_BW);
    LoRa.setTxPower(LORA_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);
    // LoRa.setGain(0);    // Supported values are between 0 and 6. If gain is 0, AGC will be enabled and LNA gain will not be used.
                        // Else if gain is from 1 to 6, AGC will be disabled and LNA gain will be used.

    LoRa.enableCrc();

    flood_init();
    flood_set_rx_handler(on_flood_receive);
    neighbor_set_update_handler(on_neighbor_update);

    term_println("[LORA] Starting LoRa ok");

    next_report_millis = millis() + LORA_REPORT_PERIOD_INIT;
}

// ----------------------------------------------------------------------------
void lora_reporting_process() {
    if (millis() > next_report_millis) {
        report_status_to(SINK_ADDRESS);
        next_report_millis = millis() + LORA_REPORT_PERIOD;
    }
}

// ----------------------------------------------------------------------------
void test_routing_send_to_zero() {
    if (getAddress() == SINK_ADDRESS)
        return;

    static uint32_t next = millis() + 10000 + ((rand() & 0b011) << 10);
    if (millis() > next) {
        flood_send_to(SINK_ADDRESS, "Hello", 6);
        next = millis() + 10000 + ((rand() & 0b011) << 10);
    }
}
