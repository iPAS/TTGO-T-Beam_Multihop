#ifndef __FLOOD_H__
#define __FLOOD_H__


#include "all_headers.h"
#include "commqueue.h"


#define FLOOD_MSG_TYPE  0x01
#define REPORT_MSG_TYPE 0x22

#define MAX_HOP         255
#define WAIT_PARENT     10000

#define MAX_HISTORY     16  // XXX: maximum node sources that can exist.
#define MAX_NEIGHBOR    6   // Max kept information of neighbors.

typedef struct __attribute__((packed))
{
    uint8_t seqNo;
    uint8_t hopCount;
    Address originSource;
    Address finalSink;
} RoutingHeader;

typedef void (*on_rx_sink)(void *message, uint8_t len);

extern void flood_init(void);
extern void flood_set_rx_handler(on_rx_sink fn);
extern bool flood_send_to(Address sink, const void *msg, uint8_t len);
extern bool flood_send_status_to(Address sink);


#endif  // __FLOOD_H__
