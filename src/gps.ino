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

#define GPS_STAMP_PERIOD 60000 * 1  // One minutes
#define GPS_REPORT_PERIOD 60000 * 10  // Five minutes
#define GPS_REPORT_PERIOD_INIT 60000 * 1

static TinyGPSPlus gps;
static uint32_t next_gps_stamp_millis;
static uint32_t next_gps_report_millis;
static bool gps_updated;

static char str_gps_datetime[20];
static char str_gps_loc[32];
static char str_gps_quality[10];

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

    next_gps_stamp_millis = millis();
    next_gps_report_millis = millis() + GPS_REPORT_PERIOD_INIT;
    gps_updated = false;
}

// ----------------------------------------------------------------------------
void gps_decoding_process() {
    while (SERIAL_GPS.available()) {
        gps.encode(SERIAL_GPS.read());
    }

    // --------------------
    // Print time cyclingly
    // --------------------
    static char buf[sizeof(str_gps_datetime) + sizeof(str_gps_loc) + sizeof(str_gps_quality) + 10];

    if (millis() > next_gps_stamp_millis) {

        if (gps.satellites.isValid() && gps.time.isUpdated() && gps.location.isValid()) {
            // Example: http://arduiniana.org/libraries/tinygpsplus/
            snprintf(str_gps_datetime, sizeof(str_gps_datetime), "%02u-%02u-%04u %02u:%02u:%02u",
                gps.date.day(),  gps.date.month(),  gps.date.year(), gps.time.hour(), gps.time.minute(), gps.time.second());
            snprintf(str_gps_loc, sizeof(str_gps_loc), "(%f,%f,%.2f)", gps.location.lat(), gps.location.lng(), gps.altitude.meters());
            snprintf(str_gps_quality, sizeof(str_gps_quality), "Sat:%d", gps.satellites.value());

            snprintf(buf, sizeof(buf), "[GPS] %s, %s, %s", str_gps_datetime, str_gps_loc, str_gps_quality);
            term_println(buf);

            gps_updated = true;
        }

        next_gps_stamp_millis = millis() + GPS_STAMP_PERIOD;
    }

    if (millis() > next_gps_report_millis  &&  gps_updated) {
        if (flood_send_to(SINK_ADDRESS, buf, strlen(buf))) {  // Not send NULL.
            term_println("[GPS] Reporting failed!");
        }

        gps_updated = false;
        next_gps_report_millis = millis() + GPS_REPORT_PERIOD;
    }
}
