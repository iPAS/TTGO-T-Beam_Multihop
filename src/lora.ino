#include <strings.h>

#include <SSD1306.h>

#include <SPI.h>
#include <LoRa.h>

#include "all_headers.h"
#include "neighbor.h"
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
bool send_status_to(Address sink) {
    neighbor_status_t statuses[MAX_NEIGHBOR];
    neighbor_status_t *sts = statuses;
    neighbor_t *nb = neighbor_table();
    uint8_t i, cnt;
    for (i = 0, cnt = 0; i < MAX_NEIGHBOR; i++, nb++)
    {
        if (nb->addr != BROADCAST_ADDR)
        {
            sts->addr = nb->addr;
            sts->rssi = nb->rssi;
            sts->snr = nb->snr;
            sts++;
            cnt++;
        }
    }
    cnt *= sizeof(neighbor_status_t);

    term_printf("[LORA] Report status node %d to %d, %d bytes", getAddress(), sink, cnt);

    if (getAddress() == sink)
        return true;  // 'sink' needs none transaction.

    return flood_send_to(sink, statuses, cnt);
}

// ----------------------------------------------------------------------------
bool report_status_to(Address sink) {
    char buf[100];  // Guess max payload as per LoRa packet. Max = sizeof(buf)-1
    char *p = buf;
    uint8_t i, cnt;
    neighbor_t *nb = neighbor_table();
    for (i = 0, cnt = 0; i < MAX_NEIGHBOR; i++, nb++)
    {
        if (nb->addr != BROADCAST_ADDR)
        {
            uint8_t len = snprintf(p, sizeof(buf)-cnt, "#%d:%d,%.2f\n", nb->addr, nb->rssi, nb->snr);
            p += len;
            cnt += len;
            if (cnt >= sizeof(buf)-1)
            {
                sprintf(p-3, "...");  // 'more' indicator
                break;
            }
        }
    }

    term_printf("[LORA] Report status node %d to %d, %d bytes:", getAddress(), sink, cnt);
    term_print(buf);
    term_println("[/LORA]");

    if (getAddress() == sink)
        return true;  // 'sink' needs none transaction.

    return flood_send_to(sink, buf, cnt);  // Not send NULL.
}

// ----------------------------------------------------------------------------
bool report_gps_to(Address sink) {
    char *str = gps_update_str("%s\n%s\n%s\n");
    return flood_send_to(SINK_ADDRESS, str, strlen(str));
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

    term_println("[DEBUG] Starting LoRa ok");

    next_report_millis = millis() + LORA_REPORT_PERIOD_INIT;
}

// ----------------------------------------------------------------------------
void lora_reporting_process() {
    if (millis() > next_report_millis) {
        // send_status_to(SINK_ADDRESS);  // XXX: as binary
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
