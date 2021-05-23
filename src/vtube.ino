#include "all_headers.h"
#include "flood.h"


// pyserial-miniterm /dev/ttyUSB0 38400 --parity E --eol CRLF
#define EOL "\r\n"

#define SERIAL_V            Serial2
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

static const char *weather_station_commands[] = {  // TODO: assign commands to be used
    "$ sys print_off",
};


// ----------------------------------------------------------------------------
void vtube_setup() {
    SERIAL_V.begin(VTUBE_UART_BAUDRATE, VTUBE_UART_CONFIG, VTUBE_RX, VTUBE_TX);
    SERIAL_V.setRxBufferSize(VTUBE_RX_BUFFER_SIZE);
    SERIAL_V.setTimeout(VTUBE_UART_TMO);
    while (!SERIAL_V)
        vTaskDelay(0);  // Yield
    vtube_command_to_station(weather_station_commands[0]);  // Set non-verbose to the weather station.
    while (SERIAL_V.available()) 
        SERIAL_V.read();  // Clear buffer

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

            // Transmit to node #0
            if (flood_send_to(0, buffer.c_str(), buffer.length()) == false) {
                Serial.println("[VTUBE] flood_send_to() error");
            }

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


    // ----------------------------------------
    // TODO: Query the weather station if not node #0
    // ----------------------------------------

    if (getAddress() != SINK_ADDRESS) {

    }
}


void vtube_command_to_station(String cmd) {
    SERIAL_V.print(cmd + EOL);  // The weather station needs the end-of-line symbol as '\r\n'.
}


void test_vtube_loopback() {
    while (true) {
        if (SERIAL_V.available()) {
            String input = SERIAL_V.readStringUntil('\n');
            Serial.print("[VTUBE] Recv: ");
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
