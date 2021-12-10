#include "all_headers.h"
#include "flood.h"

#include <strings.h>
#include <SimpleCLI.h>


SimpleCLI cli;
static Command cmd_help;
static Command cmd_node_id;
static Command cmd_vtube;
static Command cmd_flood_send;
static Command cmd_status_report;
static Command cmd_gps_report;
static Command cmd_axp_report;
static Command cmd_flood_ping;

// ----------------------------------------------------------------------------
static boolean is_numeric(String str) {
    // http://tripsintech.com/arduino-isnumeric-function/
    unsigned int stringLength = str.length();

    if (stringLength == 0) {
        return false;
    }

    boolean seenDecimal = false;

    for(unsigned int i = 0; i < stringLength; ++i) {
        if (isDigit(str.charAt(i))) {
            continue;
        }

        if (str.charAt(i) == '.') {
            if (seenDecimal) {
                return false;
            }
            seenDecimal = true;
            continue;
        }
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
static bool extract_id(Argument arg, long *ret) {
    String value = arg.getValue();
    long id = value.toInt();

    if (id == 0) {
        if (is_numeric(value)) {  // Re-check
            *ret = id;
            return true;
        }
    }
    else
    if (id > 0 && id < 65536) {
        *ret = id;
        return true;
    }

    return false;
}

// ----------------------------------------------------------------------------
void on_error_callback(cmd_error *e) {
    CommandError cmdError(e); // Create wrapper object
    term_println("[CLI] " + cmdError.toString());
    if (cmdError.hasCommand()) {
        term_printf("[CLI] Did you mean '%s' ?", cmdError.getCommand().toString().c_str());
    }
}

// ----------------------------------------------------------------------------
void on_cmd_help(cmd *c) {
    const char *desc[] = {
        "\thelp",
        "\tnode_id [new_id] -- set/get id (BROADCAST_ADDR for built-in)",
        "\tvtube ... -- send following through VTube port",
        "\tsend [sink_id] -- send to sink for testing [default 0]",
        "\tstatus [sink_id] -- send status report to sink [default 0]",
        "\tgps [sink_id] -- send GPS report to sink [default 0]",
        "\taxp [sink_id] -- send AXP report to sink [default 0]",
        "\tping [sink_id] -- ping to sink for testing [default 0]",
    };
    uint8_t i;
    Command cmd(c);
    term_println("[CLI] help:");
    for (i = 0; i < sizeof(desc)/sizeof(desc[0]); i++) {
        term_println(desc[i]);
    }
}

// ----------------------------------------------------------------------------
void on_cmd_node_id(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("id");
    long id;
    bool legal_id = extract_id(arg, &id);

    if (arg.isSet()) {  // The argument is provided.
        if (legal_id) {
            // Set new id
            term_printf("[CLI] node_id: new id %d", setAddress(id));
            oled_update_display();
            config_save(R_NODE_ID);
        }
        else {
            // Illegal id
            term_println("[CLI] node_id: illegal id");
        }

    }
    else {  // No argument
        // Ask for current id
        term_printf("[CLI] node_id: current id %d", getAddress());
    }
}

// ----------------------------------------------------------------------------
void on_cmd_vtube(cmd *c) {
    Command cmd(c);
    String arg = cmd.getArg(0).getValue();
    term_println("[CLI] vtube: '" + arg + "'");
    vtube_command_to_station(arg);
}

// ----------------------------------------------------------------------------
void on_cmd_flood_send(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("id");
    long id;
    bool legal_id = extract_id(arg, &id);

    if (legal_id == false) {
        term_println("[CLI] send: illegal sink id");  // Illegal id
    }
    else {
        const char msg[] = "hello\n";
        if (flood_send_to(id, msg, sizeof(msg)-1)) {
            term_printf("[CLI] send: '%s' to %d", msg, id);
        }
        else {
            term_println("[CLI] send: flood_send_to() failed!");
        }
    }
}

// ----------------------------------------------------------------------------
void on_cmd_status_report(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("id");
    long id;
    bool legal_id = extract_id(arg, &id);

    if (legal_id == false) {
        term_println("[CLI] status: illegal sink id");  // Illegal id
    }
    else {
        if (report_status_to(id)) {
            term_printf("[CLI] status: report to %d", id);
        }
        else {
            term_println("[CLI] status: report_status_to() failed!");
        }
    }
}

// ----------------------------------------------------------------------------
void on_cmd_gps_report(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("id");
    long id;
    bool legal_id = extract_id(arg, &id);

    if (legal_id == false) {
        term_println("[CLI] gps: illegal sink id");  // Illegal id
    }
    else {
        gps_update_data();
        if (report_gps_to(id)) {
            term_printf("[CLI] gps: report to %d", id);
        }
        else {
            term_println("[CLI] gps: report_gps_to() failed!");
        }
    }
}

// ----------------------------------------------------------------------------
void on_cmd_axp_report(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("id");
    long id;
    bool legal_id = extract_id(arg, &id);

    if (legal_id == false) {
        term_println("[CLI] axp: illegal sink id");  // Illegal id
    }
    else {
        axp_update_data();
        if (report_axp_to(id)) {
            term_printf("[CLI] axp: report to %d", id);
        }
        else {
            term_println("[CLI] axp: report_axp_to() failed!");
        }
    }
}

// ----------------------------------------------------------------------------
void on_cmd_flood_ping(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("id");
    long id;
    bool legal_id = extract_id(arg, &id);

    if (legal_id == false) {
        term_println("[CLI] ping: illegal sink id");  // Illegal id
    }
    else {
        const char msg[] = "ping\n";
        if (flood_send_to(id, msg, sizeof(msg)-1)) {
            term_printf("[CLI] ping: '%s' to %d", msg, id);
        }
        else {
            term_println("[CLI] ping: flood_send_to() failed!");
        }
    }
}

// ----------------------------------------------------------------------------
void cli_setup() {
    Serial.begin(115200);
    while (!Serial)
        vTaskDelay(1);  // Yield

    cli.setOnError(on_error_callback); // Set error Callback

    cmd_help = cli.addCommand("help", on_cmd_help);
    cmd_node_id = cli.addCommand("node_id", on_cmd_node_id);
    cmd_node_id.addPositionalArgument("id", "");  // Default value is ""
    cmd_vtube = cli.addSingleArgumentCommand("vtube", on_cmd_vtube);
    cmd_flood_send = cli.addCommand("send", on_cmd_flood_send);
    cmd_flood_send.addPositionalArgument("id", "0");  // Default value is "0"
    cmd_status_report = cli.addCommand("status", on_cmd_status_report);
    cmd_status_report.addPositionalArgument("id", "0");  // Default value is "0"
    cmd_gps_report = cli.addCommand("gps", on_cmd_gps_report);
    cmd_gps_report.addPositionalArgument("id", "0");  // Default value is "0"
    cmd_axp_report = cli.addCommand("axp", on_cmd_axp_report);
    cmd_axp_report.addPositionalArgument("id", "0");  // Default value is "0"
    cmd_flood_ping = cli.addCommand("ping", on_cmd_flood_ping);
    cmd_flood_ping.addPositionalArgument("id", "0");  // Default value is "0"
}

// ----------------------------------------------------------------------------
void cli_interpreting_process() {
    static String line = "";

    while (Serial.available()) {
        if (Serial.peek() == '\n'  ||  Serial.peek() == '\r') {
            Serial.read();  // Just ignore
            if (line.length() > 0) {
                cli.parse(line);  // Parse the user input into the CLI
                line = "";
                break;
            }
        }
        else {
            line += (char)Serial.read();
        }
    }
}
