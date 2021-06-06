#include <axp20x.h>

#include "all_headers.h"


#define AXP_SDA 21
#define AXP_SCL 22
#define AXP_IRQ 35

static AXP20X_Class axp;

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

    return is_tbeam_version_less_v1;
}
