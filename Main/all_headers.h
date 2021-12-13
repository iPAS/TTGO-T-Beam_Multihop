#ifndef __ALL_HEADERS_H__
#define __ALL_HEADERS_H__


#include <Arduino.h>
#include "bind.h"


// ------------
// From cli.ino
// ------------
extern const char * remote_commands[];
#define REMOTE_CMD_HELLO    remote_commands[0]
#define REMOTE_CMD_PING     remote_commands[1]
#define REMOTE_CMD_RESET    remote_commands[2]

extern const char * remote_responses[];
#define REMOTE_RESP_PING    remote_responses[1]


#endif  // __ALL_HEADERS_H__
