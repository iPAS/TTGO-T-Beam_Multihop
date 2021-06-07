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

#define VTUBE_BATCH_SIZE        60
#define VTUBE_BATCH_PERIOD      60000

#define VTUBE_CMD_GAP           4000
#define VTUBE_CMD_PERIOD        (60000*5)  // Five minute
#define VTUBE_CMD_PERIOD_INIT   10000

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

// ----------------------------------------------------------------------------
void vtube_setup() {
    SERIAL_V.begin(VTUBE_UART_BAUDRATE, VTUBE_UART_CONFIG, VTUBE_RX, VTUBE_TX);
    SERIAL_V.setRxBufferSize(VTUBE_RX_BUFFER_SIZE);
    SERIAL_V.setTimeout(VTUBE_UART_TMO);
    while (!SERIAL_V)
        vTaskDelay(1);  // Yield
    while (SERIAL_V.available())
        SERIAL_V.read();  // Clear buffer

    vtube_command_to_station(cmd_quiet);  // Set non-verbose to the weather station.

    buffer = "";
    next_batch_millis = millis() + VTUBE_BATCH_PERIOD;
    next_cmd_millis = millis() + VTUBE_CMD_PERIOD_INIT;
    count_cmd_sent = 0;
}

// ----------------------------------------------------------------------------
void vtube_forwarding_process() {
    if (getAddress() == SINK_ADDRESS)
        return;

    static String line, sub;
    int16_t i, j;

    // ------------------------------------
    // Aggregate data into a bulk then send
    // ------------------------------------
    if (SERIAL_V.available()) {
        line = SERIAL_V.readStringUntil('\n');  // The terminator character is discarded from the serial buffer.
                                                // '\n' NOT included.

        // Filter-out unused information
        for (i = 0; i < line.length();) {       // Cut some response
            j = line.indexOf('\r', i);          // Separated with '\r'
            if (j < 0) break;
            if (j > i) {                        // Ignore a single '\r'.
                sub = line.substring(i, j);     // @ [j] NOT included
                // term_printf("  %d,%d\t%s", i, j-1, sub.c_str());  // XXX: for debugging

                // Filtering mechanism
                if (sub[0] == '$') 
                    ;  // Skip echo
                else
                    buffer += sub + '\n';
            }
            i = j+1;  // Next char left
        }

        next_batch_millis = millis() + VTUBE_BATCH_PERIOD;  // Reset timeout
    }

    // -------------
    // Send the bulk
    // -------------
    if(buffer.length() > VTUBE_BATCH_SIZE  ||  millis() > next_batch_millis) {
        if (buffer.length() > 0) {
            // Pass 'input' through LoRa network to the node #0 by putting them into sedning queue.
            term_printf("[VTUBE] To %d:", SINK_ADDRESS);


            // for (i = 0; i < buffer.length(); i++) {  // XXX: for debugging
            //     if (buffer[i] == 0x0D  ||  buffer[i] == 0x0A) {
            //         term_println();
            //         term_println(buffer[i], HEX);
            //     }
            //     else {
            //         term_print(buffer[i], HEX);
            //         term_print(' ');
            //     }
            // }
            // term_println();


            for (i = 0; i < buffer.length();) {  // XXX: for debugging
                j = buffer.indexOf('\n', i);
                if (j < 0) break;
                sub = buffer.substring(i, j);
                term_printf("  %d,%d\t%s", i, j-1, sub.c_str());
                i = j+1;  // Next char left
            }


            // Transmit to node 'SINK_ADDRESS'
            if (flood_send_to(SINK_ADDRESS, buffer.c_str(), buffer.length()) == false) {
                term_println("[VTUBE] flood_send_to() error");
            }

            buffer = "";  // Clear the transmitted bulk.
        }

        next_batch_millis = millis() + VTUBE_BATCH_PERIOD;
    }

    // -------------------------
    // Query the weather station
    // -------------------------
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

// ----------------------------------------------------------------------------
void vtube_command_to_station(String cmd) {
    SERIAL_V.print(cmd + EOL);  // The weather station needs the end-of-line symbol as '\r\n'.

    term_printf("[VTUBE] Send cmd: '%s'", cmd.c_str());
}

// ----------------------------------------------------------------------------
void test_vtube_loopback() {
    while (true) {
        if (SERIAL_V.available()) {
            String input = SERIAL_V.readStringUntil('\n');
            term_print("[VTUBE] Recv: ");
            term_println(input);
        }

        if (Serial.available()) {
            String input = Serial.readStringUntil('\n');
            term_print("[VTUBE] Send: ");
            term_println(input);
            SERIAL_V.println(input);
        }
    }
}
