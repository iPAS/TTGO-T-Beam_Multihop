/**
 * @file ota.ino
 * @author Pasakorn Tiwatthanont (ptiwatthanont@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-03-29
 *
 * https://randomnerdtutorials.com/esp32-access-point-ap-web-server/
 * https://lastminuteengineers.com/esp32-ota-updates-arduino-ide/
 */
#include "all_headers.h"

#include <strings.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


static char wifi_ssid[10];
static char wifi_passwd[10];

// ----------------------------------------------------------------------------
void ota_setup() {
    //
    snprintf(wifi_ssid, sizeof(wifi_ssid), "lrrl%d", getAddress());
    snprintf(wifi_passwd, sizeof(wifi_passwd), "12345678");
    WiFi.softAP(wifi_ssid, wifi_passwd);
    IPAddress ip_addr = WiFi.softAPIP();

    ArduinoOTA
    .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        term_println("[OTA] Start updating " + type);
    })
    .onEnd([]() {
        term_println("\n[OTA] End");
    })
    // .onProgress([](unsigned int progress, unsigned int total) {
    //     term_printf("[OTA] .. %u%%\n", (progress / (total / 100)));
    // })
    .onError([](ota_error_t error) {
        String msg;
        msg =   (error == OTA_AUTH_ERROR)? "Auth Failed" :
                (error == OTA_BEGIN_ERROR)? "Begin Failed" :
                (error == OTA_CONNECT_ERROR)? "Connect Failed" :
                (error == OTA_RECEIVE_ERROR)? "Receive Failed" :
                (error == OTA_END_ERROR)? "End Failed" : "";
        term_printf("[OTA] Error[%u]: %s", error, msg.c_str());
    });

    ArduinoOTA.setTimeout(20000);
    ArduinoOTA.begin();

    term_printf("[OTA] Starting SoftAP ok @ ssid:%s passwd:%s ip:%s",
            wifi_ssid, wifi_passwd, ip_addr.toString().c_str());
}

// ----------------------------------------------------------------------------
void ota_process() {
    ArduinoOTA.handle();
}
