#ifndef __NEIGHBOR_H__
#define __NEIGHBOR_H__


#include "all_headers.h"
#include "flood.h"


#ifdef NEIGHBOR_T
typedef struct
{
    Address addr;
    uint32_t timestamp;

    uint8_t rssi;
} neighbor_t;
#endif

extern void neighbor_init();
extern neighbor_t *neighbor_find(Address addr);

typedef void (*on_nb_update)(neighbor_t *nb);

extern void neighbor_set_update_handler(on_nb_update callback);
extern void neighbor_update_info(Address source);

extern neighbor_t *neighbor_table();


#endif  // __NEIGHBOR_H__
