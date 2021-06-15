#include "all_headers.h"

#include <axp20x.h>


#define AXP_LOG_PERIOD 60000 * 15
#define AXP_LOG_PERIOD_INIT 45000

#define AXP_SDA 21
#define AXP_SCL 22
#define AXP_IRQ 35

static AXP20X_Class axp;
static uint32_t next_log_millis;

// ----------------------------------------------------------------------------
bool axp_setup() {
    bool is_tbeam_version_less_v1;

    Wire.begin(AXP_SDA, AXP_SCL);
    if (axp.begin(Wire, AXP192_SLAVE_ADDRESS) == AXP_FAIL) {
        term_println("[DEBUG] Starting AXP192 failed! -- guessing, this is the V0.7");
        is_tbeam_version_less_v1 = true;
    } else {
        term_println("[DEBUG] Starting AXP192 succeeded! -- guessing, its version >= V1.0");

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

    next_log_millis = millis() + AXP_LOG_PERIOD_INIT;

    return is_tbeam_version_less_v1;
}

// ----------------------------------------------------------------------------
void axp_logging_process() {
    if (millis() > next_log_millis) {
        axp.getTemp();
    // axp.getAcinVoltage();
    // axp.getAcinCurrent();
    // axp.getVbusVoltage();
    // axp.getVbusCurrent();

    // float       getBattInpower(void);
    // float       getBattVoltage(void);
    // float       getBattChargeCurrent(void);
    // float       getBattDischargeCurrent(void);
    // uint32_t    getBattChargeCoulomb(void);
    // uint32_t    getBattDischargeCoulomb(void);

        next_log_millis = millis() + AXP_LOG_PERIOD;
    }
}

// ----------------------------------------------------------------------------
void axp_update_data() {
    // snprintf(str_gps_datetime, sizeof(str_gps_datetime), "%02u-%02u-%04u %02u:%02u:%02u",
    //     gps.date.day(),  gps.date.month(),  gps.date.year(),
    //     gps.time.hour(), gps.time.minute(), gps.time.second());
    // snprintf(str_gps_loc, sizeof(str_gps_loc), "(%f,%f,%.2f)",
    //     gps.location.lat(), gps.location.lng(), gps.altitude.meters());
    // snprintf(str_gps_quality, sizeof(str_gps_quality), "Sat:%d",
    //     gps.satellites.value());
}
