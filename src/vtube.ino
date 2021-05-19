#include "all_headers.h"


// screen /dev/ttyUSB1 38400,cs8,parenb,-parodd
#define SERIAL_V Serial2
#define VTUBE_UART_BAUDRATE  38400
#define VTUBE_UART_CONFIG    SERIAL_8E1
#define VTUBE_TX 2
#define VTUBE_RX 13


// ----------------------------------------------------------------------------
void vtube_setup() {
    SERIAL_V.begin(VTUBE_UART_BAUDRATE, VTUBE_UART_CONFIG, VTUBE_RX, VTUBE_TX);
    while (!SERIAL_V);
    while (SERIAL_V.available()) {
        SERIAL_V.read();
    }
}


void vtube_forwarding_process() {
    if (SERIAL_V.available()) {
        String input = SERIAL_V.readStringUntil('\n');

        // TODO: Pass 'input' through LoRa network to node id 0
        Serial.print("[VTUBE] >> ");
        Serial.println(input);
    }
}


void vtube_command_to_station(String cmd) {
    SERIAL_V.println(cmd);
}


void test_vtube_loopback() {
    while (true) {
        if (SERIAL_V.available()) {
            String input = SERIAL_V.readStringUntil('\n');
            Serial.println(input);
        }

        if (Serial.available()) {
            String input = Serial.readStringUntil('\n');
            Serial.println("Send >>>");
            SERIAL_V.println(input);
        }
    }
}
