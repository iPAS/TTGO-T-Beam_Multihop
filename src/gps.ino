#include <strings.h>
#include <TinyGPS++.h>

#include "all_headers.h"


#define SERIAL_GPS Serial1
#define GPS_BAUDRATE 9600

static uint8_t gps_tx;
static uint8_t gps_rx;

#define GPS_TX_V07 15
#define GPS_RX_V07 12

#define GPS_TX_V10 12
#define GPS_RX_V10 34

#define GPS_REPORT_PERIOD 60000 * 1  // One munites

static TinyGPSPlus gps;
static uint32_t next_stamp_millis;

// ----------------------------------------------------------------------------
void gps_setup(bool is_tbeam_version_less_v1) {
    if (is_tbeam_version_less_v1) {
        gps_tx = GPS_TX_V07;
        gps_rx = GPS_RX_V07;
    } else {
        gps_tx = GPS_TX_V10;
        gps_rx = GPS_RX_V10;
    }

    SERIAL_GPS.begin(GPS_BAUDRATE, SERIAL_8N1, gps_rx, gps_tx);
    while (!SERIAL_GPS)
        vTaskDelay(1);  // Yield
    while (SERIAL_GPS.available())
        SERIAL_GPS.read();  // Clear buffer

    next_stamp_millis = millis();
}

// ----------------------------------------------------------------------------
void gps_decoding_process() {
    while (SERIAL_GPS.available()) {
        gps.encode(SERIAL_GPS.read());
    }

    // --------------------
    // Print time cyclingly
    // --------------------
    if (millis() > next_stamp_millis) {

        if (gps.satellites.isValid() && gps.time.isUpdated() && gps.location.isValid()) {
            // Example: http://arduiniana.org/libraries/tinygpsplus/
            static char buf[200];
            snprintf(buf, sizeof(buf),
                "[GPS] %02u-%02u-%04u %02u:%02u:%02u, "
                "SAT %d, LAT %f, LON %f, ALT %f, "
                "RSSI %f, SNR %f",
                gps.date.day(),  gps.date.month(),  gps.date.year(),
                gps.time.hour(), gps.time.minute(), gps.time.second(),
                gps.satellites.value(), gps.location.lat(), gps.location.lng(), gps.altitude.meters(),
                LoRa.packetRssi(), LoRa.packetSnr());

            String s_buf(buf);
            term_println(s_buf);
        }

        next_stamp_millis = millis() + GPS_REPORT_PERIOD;
    }
}
