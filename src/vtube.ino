#include "all_headers.h"
#include "flood.h"


// pyserial-miniterm /dev/ttyUSB0 38400 --parity E --eol CRLF
#define EOL "\r\n"

#define SERIAL_V            Serial2
#define VTUBE_UART_BAUDRATE 38400  // Determined by the weather station RTU
#define VTUBE_UART_CONFIG   SERIAL_8E1
#define VTUBE_TX 2
#define VTUBE_RX 13

#define VTUBE_RX_BUFFER_SIZE    512
#define VTUBE_UART_TMO          3000
#define VTUBE_BATCH_PERIOD      30000
#define VTUBE_BATCH_SIZE        60

#define VTUBE_CMD_GAP           2000
#define VTUBE_CMD_PERIOD        60000 * 2  // Two minute

static String buffer;
static uint32_t next_batch_millis, next_cmd_millis;
static uint8_t count_cmd_sent;

static const char *ws_commands[] = {  // weather_station_commands
    "$ sys print_off",      // $ sys print_off | disable print message
    "$ route get_nid",      // $ route get_nid | get node ID and network ID
    "$ rtc get 1",          // $ rtc get 1     | get RTC
                            // $ rtc set #d #t | update RTC with host RTC
                            // $ pwrsw on 330  | on sensor power 3.3V
    "$ i2c get 1099",       // $ i2c get 1099  | read T/H intf
    "$ wind get 0",         // $ wind get 0    | get wind speed and direction (update every 10 sec.)
    "$ rain get 0",         // $ rain get 0    | get rain volumn(CC)
    "$ landsld get 0",      // $ landsld get 0 | get rain volumn24Hr window
    "$ uc20 get",           // $ uc20 get      | get GSM signal
    "$ charger get",        // $ charger get   | get Vbatt
    "$ atod get 22",        // $ atod get 22   | get Charging Current
    "$ atod get 20",        // $ atod get 20   | get Bus Voltage
};

static const char *cmd_quiet  = ws_commands[0];
// static const char *cmd_nodeid = ws_commands[1];
// static const char *cmd_rtc    = ws_commands[2];


// ----------------------------------------------------------------------------
void vtube_setup() {
    SERIAL_V.begin(VTUBE_UART_BAUDRATE, VTUBE_UART_CONFIG, VTUBE_RX, VTUBE_TX);
    SERIAL_V.setRxBufferSize(VTUBE_RX_BUFFER_SIZE);
    SERIAL_V.setTimeout(VTUBE_UART_TMO);
    while (!SERIAL_V)
        vTaskDelay(0);  // Yield
    vtube_command_to_station(cmd_quiet);  // Set non-verbose to the weather station.
    while (SERIAL_V.available()) 
        SERIAL_V.read();  // Clear buffer

    buffer = "";
    next_batch_millis = millis() + VTUBE_BATCH_PERIOD;
    next_cmd_millis = millis() + VTUBE_CMD_PERIOD;
    count_cmd_sent = 0;
}


void vtube_forwarding_process() {
    // ------------------------------------
    // Aggregate data into a bulk then send
    // ------------------------------------
    if (SERIAL_V.available()) {
        buffer += SERIAL_V.readStringUntil('\n');  // The terminator character is discarded from the serial buffer.
        buffer += '\n';  // As a separator.

        next_batch_millis = millis() + VTUBE_BATCH_PERIOD;  // Reset timeout
    }

    // -------------
    // Send the bulk
    // -------------
    if(buffer.length() > VTUBE_BATCH_SIZE  ||  millis() > next_batch_millis) {
        if (buffer.length() > 0) {
            // Pass 'input' through LoRa network to the node #0 by putting them into sedning queue.
            Serial.print("[VTUBE] To ");
            Serial.print(SINK_ADDRESS);
            Serial.println(": ");

            String buffer_sent = "";  // 'buffer' to be sent actually.
            String line;  // The response from each previous command.

            // Filter-out unused information & debug the bulk by testing to extract
            int16_t i, j;
            for (i = 0; i < buffer.length();) {
                j = buffer.indexOf('\n', i);
                if (j < 0) break;

                line = buffer.substring(i, j-1);  // -1 for skipping '\n'

                Serial.print("  ");
                Serial.print(i);
                Serial.print(',');
                Serial.print(j-1);
                Serial.println("\t" + line);

                i = j+1;  // Skip '\n' and the last char.

                // TODO: Filtering mechanism
                buffer_sent += line + '\n';
            }
            Serial.println();

            // Transmit to node 'SINK_ADDRESS'
            if (flood_send_to(SINK_ADDRESS, buffer_sent.c_str(), buffer_sent.length()) == false) {
                Serial.println("[VTUBE] flood_send_to() error");
            }

            buffer = "";  // Clear the transmitted bulk.
        }

        next_batch_millis = millis() + VTUBE_BATCH_PERIOD;
    }

    // -----------------------------------------------------------
    // Query the weather station if not node #0 & empty any return
    // -----------------------------------------------------------
    if (getAddress() != SINK_ADDRESS)
        if (millis() > next_cmd_millis) {
            if (count_cmd_sent < sizeof(ws_commands)/sizeof(ws_commands[0])) {
                vtube_command_to_station(ws_commands[count_cmd_sent]);

                next_cmd_millis = millis() + VTUBE_CMD_GAP;
                count_cmd_sent++;
            }
            else {
                next_cmd_millis = millis() + VTUBE_CMD_PERIOD;
                count_cmd_sent = 0;
            }
        }
}


void vtube_command_to_station(String cmd) {
    SERIAL_V.print(cmd + EOL);  // The weather station needs the end-of-line symbol as '\r\n'.

    Serial.print("[VTUBE] Send cmd: '");
    Serial.print(cmd);
    Serial.println("'");
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
