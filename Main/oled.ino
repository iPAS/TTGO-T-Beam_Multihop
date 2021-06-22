#include "all_headers.h"

#include <SSD1306.h>


static SSD1306 display(0x3C, 21, 22);

// ----------------------------------------------------------------------------
void oled_setup() {
    display.init();
    display.flipScreenVertically();

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    // delay(100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    oled_update_display();
}

// ----------------------------------------------------------------------------
void oled_update_display() {
    display.clear();

    // display.drawString(0, 0, rssi + ", " + snr);
    // display.drawString(0, 15, "Recv: " + packSize + " bytes");
    // display.drawStringMaxWidth(0, 26, 128, packet);

    display.drawString(0, 0, String("Address: ") + getAddress());

    display.display();
}
