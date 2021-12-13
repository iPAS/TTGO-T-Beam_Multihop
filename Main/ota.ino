#include "all_headers.h"

#include <SPIFFS.h>


// ----------------------------------------------------------------------------
void ota_setup() {
    if (!SPIFFS.begin(true)) {
        term_println("[OTA] Starting SPIFFS failed!");
        while (1);
    }

    term_println("[DEBUG] Starting OTA ok");
}
