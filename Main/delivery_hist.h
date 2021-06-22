#ifndef __DELIVERY_HIST_H__
#define __DELIVERY_HIST_H__


#include "all_headers.h"
#include "flood.h"


typedef struct
{
    bool available;
    uint32_t timestamp;

    RoutingHeader latestHdr;  // Received header.
    Address parent;         // Intermediate node who sent us. Our way back to the source.
    uint8_t currSeqNo;      // Current 'seqNo' corresponding with the 'originSource'.
} delivery_history_t;

extern void hist_init();
extern delivery_history_t *hist_find(RoutingHeader *hdr);


#endif  // __DELIVERY_HIST_H__
