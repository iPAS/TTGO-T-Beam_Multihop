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

#define GPS_REPORT_PERIOD 60000 * 5  // Five munites

static TinyGPSPlus gps;
static String gps_datetime = "@ --";
static String gps_sat      = "SAT --";
static String gps_loc      = "LAT --, LON --, ALT --";
static String gps_rssi     = "RSSI --";
static String gps_snr      = "SNR --";

static uint32_t next_report_millis;


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

    next_report_millis = millis();
}


void gps_decoding_process() {
    while (SERIAL_GPS.available()) {
        gps.encode(SERIAL_GPS.read());
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

        gps_sat = "SAT " + String(gps.satellites.value());

        gps_loc =                  "LAT " + String(gps.location.lat(), 6);
        gps_loc = gps_loc + ", " + "LON " + String(gps.location.lng(), 6);
        gps_loc = gps_loc + ", " + "ALT " + String(gps.altitude.meters());

        gps_rssi = "RSSI " + String(LoRa.packetRssi(), DEC);
        gps_snr  = "SNR "  + String(LoRa.packetSnr(), 2);
    }

    // TODO: print time cyclingly.
    if (millis() > next_report_millis) {

        
        next_report_millis = millis() + GPS_REPORT_PERIOD;
    }
}
