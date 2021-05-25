#include "all_headers.h"


#define BLINK_ON_PERIOD 200
#define BLINK_OFF_PERIOD 2000

#define LED_IO_V07 14
#define LED_IO_V10 4

static uint8_t led_io, led_active;


// ----------------------------------------------------------------------------
void led_setup(bool is_tbeam_version_less_v1) {
    led_io = (is_tbeam_version_less_v1)? LED_IO_V07 : LED_IO_V10;
    led_active = (is_tbeam_version_less_v1)? HIGH : LOW;
    pinMode(led_io, OUTPUT);
    digitalWrite(led_io, !led_active);
}


void led_blinking_process() {
    static uint8_t state = 0;
    static uint32_t next = 0;

    if (millis() > next) {
        switch (state) {
            case 0:
                digitalWrite(led_io, led_active);
                next = millis() + BLINK_ON_PERIOD;
                state = 1;
                break;

            case 1:
                digitalWrite(led_io, !led_active);
                next = millis() + BLINK_OFF_PERIOD;
                state = 0;
                break;
        }
    }
}
