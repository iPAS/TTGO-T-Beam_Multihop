#ifndef __NEIGHBOR_H__
#define __NEIGHBOR_H__


#include "all_headers.h"
#include "flood.h"

typedef struct
{
    Address addr;
    uint32_t timestamp;

    uint8_t rssi;
} neighbor_t;

extern void neighbor_init();
extern neighbor_t *neighbor_find(Address addr);
extern neighbor_t *neighbor_table();


#endif  // __NEIGHBOR_H__
