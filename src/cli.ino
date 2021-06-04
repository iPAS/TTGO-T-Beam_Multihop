#include <strings.h>
#include <SimpleCLI.h>

#include "all_headers.h"


SimpleCLI cli;
static Command cmd_help;
static Command cmd_hello;
static Command cmd_node_id;
static Command cmd_vtube;
static Command cmd_flood_send;


// ----------------------------------------------------------------------------
boolean isNumeric(String str) {
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
void on_error_callback(cmd_error *e) {
    CommandError cmdError(e); // Create wrapper object
    term_println("[CLI] " + cmdError.toString());
    if (cmdError.hasCommand()) {
        term_printf("[CLI] Did you mean '%s' ?", cmdError.getCommand().toString().c_str());
    }
}


void on_cmd_help(cmd *c) {
    const char *desc[] = {
        "\thelp",
        "\thello",
        "\tnode_id [new_id] -- set/get id (BROADCAST_ADDR for built-in)",
        "\tvtube ... -- send following through VTube port",
        "\tsend [sink_id] -- send to sink for testing [default 0]",
    };
    uint8_t i;
    Command cmd(c);
    term_println("[CLI] help:");
    for (i = 0; i < sizeof(desc)/sizeof(desc[0]); i++) {
        term_println(desc[i]);
    }
}


void on_cmd_hello(cmd *c) {
    Command cmd(c);
    term_println("[CLI] hello: Hello!");
}


void on_cmd_node_id(cmd *c) {
    Command cmd(c);
    Argument idArg = cmd.getArgument("id");
    long id = idArg.getValue().toInt();
    bool legal_id = false;

    if (idArg.isSet()) {  // The argument is provided.
        if (id == 0) {
            if (isNumeric(idArg.getValue()))  // Re-check
                legal_id = true;
        }
        else
        if (id > 0 && id < 65536) {
            legal_id = true;
        }

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


void on_cmd_vtube(cmd *c) {
    Command cmd(c);
    String arg = cmd.getArg(0).getValue();
    term_println("[CLI] vtube: '" + arg + "'");
    vtube_command_to_station(arg);
}


void on_cmd_flood_send(cmd *c) {
    Command cmd(c);
    Argument idArg = cmd.getArgument("sink");
    long id = idArg.getValue().toInt();
    bool legal_id = false;

    if (id == 0) {
        if (isNumeric(idArg.getValue()))  // Re-check
            legal_id = true;
    }
    else
    if (id > 0 && id < 65536) {
        legal_id = true;
    }

    if (legal_id == false) {
        term_println("[CLI] send: illegal sink id");  // Illegal id
    }
    else {
        const char msg[] = "hello\n";
        if (flood_send_to(id, msg, sizeof(msg)-1) == false) {
            term_println("[CLI] send: flood_send_to() error");
        }
        else {
            term_printf("[CLI] send: '%s' to %d", msg, id);
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
    cmd_hello = cli.addCommand("hello", on_cmd_hello);
    cmd_node_id = cli.addCommand("node_id", on_cmd_node_id);
    cmd_node_id.addPositionalArgument("id", "");  // Default value is ""
    cmd_vtube = cli.addSingleArgumentCommand("vtube", on_cmd_vtube);
    cmd_flood_send = cli.addCommand("send", on_cmd_flood_send);
    cmd_flood_send.addPositionalArgument("sink", "0");  // Default value is "0"
}


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
