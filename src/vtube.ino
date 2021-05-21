#include "all_headers.h"


// screen /dev/ttyUSB1 38400,cs8,parenb,-parodd
#define SERIAL_V Serial2
#define VTUBE_UART_BAUDRATE 38400  // Determined by the weather station RTU
#define VTUBE_UART_CONFIG   SERIAL_8E1
#define VTUBE_TX 2
#define VTUBE_RX 13

#define VTUBE_RX_BUFFER_SIZE    1024
#define VTUBE_UART_TMO          3000
#define VTUBE_WAIT              30000
#define VTUBE_BATCH_SIZE        60

static String buffer;
static uint32_t next;


// ----------------------------------------------------------------------------
void vtube_setup() {
    SERIAL_V.begin(VTUBE_UART_BAUDRATE, VTUBE_UART_CONFIG, VTUBE_RX, VTUBE_TX);
    SERIAL_V.setRxBufferSize(VTUBE_RX_BUFFER_SIZE);
    SERIAL_V.setTimeout(VTUBE_UART_TMO);
    while (!SERIAL_V);
    while (SERIAL_V.available()) {
        SERIAL_V.read();
    }
    buffer = "";
    next = millis() + VTUBE_WAIT;
}


void vtube_forwarding_process() {
    // ------------------------------------
    // Aggregate data into a bulk then send
    // ------------------------------------

    if (SERIAL_V.available()) {
        buffer += SERIAL_V.readStringUntil('\n');  // The terminator character is discarded from the serial buffer.
        buffer += '\n';  // As a separator.

        next = millis() + VTUBE_WAIT;  // Reset timeout
    }

    if(buffer.length() > VTUBE_BATCH_SIZE  ||  millis() > next) {
        if (buffer.length() > 0) {
            // Pass 'input' through LoRa network to the node #0 by putting them into sedning queue.
            Serial.println("[VTUBE] To #0: ");

            // Debug the bulk by testing to extract
            // Serial.println(buffer);  // All at once
            uint8_t i, j;
            for (i = 0; i < buffer.length();) {
                j = buffer.indexOf('\n', i);
                if (j < 0) break;
                String line = buffer.substring(i, j-1);  // -1 for skipping '\n'
                Serial.print(i);
                Serial.print(',');
                Serial.print(j-1);
                Serial.println("\t" + line);
                i = j+1;  // Skip '\n' and the last char.
            }
            Serial.println();

            buffer = "";  // Clear the transmitted bulk.
        }

        next = millis() + VTUBE_WAIT;
    }
}


void vtube_command_to_station(String cmd) {
    SERIAL_V.println(cmd);
}


void test_vtube_loopback() {
    while (true) {
        if (SERIAL_V.available()) {
            String input = SERIAL_V.readStringUntil('\n');
            Serial.print("[VTUBE] Got: ");
            Serial.println(input);
        }

        if (Serial.available()) {
            String input = Serial.readStringUntil('\n');
            Serial.print("[VTUBE] Send: ");
            Serial.println(input);
            SERIAL_V.println(input);
        }
    }
}
