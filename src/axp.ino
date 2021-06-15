#include "all_headers.h"

#include <strings.h>

#include <axp20x.h>


#define AXP_LOG_PERIOD 60000 * 15
#define AXP_LOG_PERIOD_INIT 45000

#define AXP_SDA 21
#define AXP_SCL 22
#define AXP_IRQ 35

static AXP20X_Class axp;
static uint32_t next_axp_log_millis;

static char str_axp_temp[10];
static char str_axp_acin[20];
static char str_axp_bus[20];
static char str_axp_bat[40];

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

        axp.ClearCoulombcounter();
        axp.EnableCoulombcounter();

        is_tbeam_version_less_v1 = false;
    }

    next_axp_log_millis = millis() + AXP_LOG_PERIOD_INIT;

    return is_tbeam_version_less_v1;
}

// ----------------------------------------------------------------------------
void axp_logging_process() {
    if (millis() > next_axp_log_millis) {
        axp_update_data();
        term_println( axp_update_str("[AXP] %s, %s, %s, %s") );

        next_axp_log_millis = millis() + AXP_LOG_PERIOD;
    }
}

// ----------------------------------------------------------------------------
void axp_update_data() {
    snprintf(str_axp_temp, sizeof(str_axp_temp), "%.2f℃", axp.getTemp());
    snprintf(str_axp_acin, sizeof(str_axp_acin), "(%.3fV,%.3fA)", axp.getAcinVoltage(), axp.getAcinCurrent());
    snprintf(str_axp_bus, sizeof(str_axp_bus), "(%.3fV,%.3fA)", axp.getVbusVoltage(), axp.getVbusCurrent());
    snprintf(str_axp_bat, sizeof(str_axp_bat), "(%.3fW,%.3fV,%.3fA,%.3fA)",
        axp.getBattInpower(), axp.getBattVoltage(), axp.getBattChargeCurrent(), axp.getBattDischargeCurrent());
    // axp.getBattChargeCoulomb();
    // axp.getBattDischargeCoulomb();
}

// ----------------------------------------------------------------------------
char *axp_update_str(const char *fmt) {
    static char str[sizeof(str_axp_temp) + sizeof(str_axp_acin) + sizeof(str_axp_bus) + sizeof(str_axp_bat) + 10];
    snprintf(str, sizeof(str), fmt, str_axp_temp, str_axp_acin, str_axp_bus, str_axp_bat);
    return str;
}
