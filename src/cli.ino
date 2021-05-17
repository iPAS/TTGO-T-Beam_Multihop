#include <strings.h>
#include <SimpleCLI.h>

#include "all_headers.h"


SimpleCLI cli;
Command cmd_hello;
Command cmd_node_id;


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


void cli_error_callback(cmd_error *e) {
    CommandError cmdError(e); // Create wrapper object
    Serial.println("[CLI] " + cmdError.toString());
}


// ----------------------------------------------------------------------------
void on_cmd_hello(cmd *c) {
    Command cmd(c);
    Serial.println("[CLI] hello: Hello!");
}


void on_cmd_node_id(cmd *c) {
    Command cmd(c);
    Argument idArg = cmd.getArgument("id");
    long id = idArg.getValue().toInt();
    bool legal_id = false;

    if (idArg.isSet()) {  // The argument is provided.
        if (id == 0) {
            if (isNumeric(idArg.getValue()))
                legal_id = true;
        } else
        if (id > 0 && id < 65536) {
            legal_id = true;
        }

        if (legal_id) {
            // Set new id
            Serial.print("[CLI] node_id: new id ");
            Serial.println(setAddress(id));
        } else {
            // Illegal id
            Serial.println("[CLI] node_id: illegal id");
        }

    } else {  // No argument
        // Ask for current id
        Serial.print("[CLI] node_id: current id ");
        Serial.println(getAddress());
    }
}


// ----------------------------------------------------------------------------
void cli_setup() {
    cli.setOnError(cli_error_callback); // Set error Callback

    cmd_hello = cli.addCommand("hello", on_cmd_hello);

    cmd_node_id = cli.addCommand("node_id", on_cmd_node_id);
    cmd_node_id.addPositionalArgument("id", "");
}


void cli_interpreting_process() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');  // Read out string from the serial monitor
        cli.parse(input);  // Parse the user input into the CLI
    }

    if (cli.errored()) {
        CommandError cmdError = cli.getError();

        Serial.print("[CLI] Error: ");
        Serial.println(cmdError.toString());

        if (cmdError.hasCommand()) {
            Serial.print("[CLI] Did you mean \"");
            Serial.print(cmdError.getCommand().toString());
            Serial.println("\"?");
        }
    }
}
