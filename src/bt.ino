#include <strings.h>

#include <BluetoothSerial.h>

#include "all_headers.h"


// ---------- Bluetooth-serial ----------
BluetoothSerial bt;
static char bt_name[20];


void bt_setup() {
    sprintf(bt_name, "lora_relay%04X", getAddress());
    bt.begin(bt_name);
}


void bt_cli_interpreting_process() {  // Process command-line input
    if (bt.connected()) {
        if (bt.available()) {
            String input = bt.readStringUntil('\n');  // Read out string from the serial monitor
            cli.parse(input);  // Parse the user input into the CLI
        }
    }
}
