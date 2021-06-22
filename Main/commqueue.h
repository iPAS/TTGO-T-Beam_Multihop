#ifndef __COMMQUEUE_H__
#define __COMMQUEUE_H__


#include "all_headers.h"
#include "queue.h"


#define DELAY_TX_DEFAULT 800
#define DELAY_TX_GAP_MASK 0x01FF

typedef struct TxTask
{
    Address dst;
    MessageType type;

    void *msg;
    uint8_t len;

    uint16_t delay_tick;
} TxTask;

extern void cq_init();
extern bool cq_send(Address dst, MessageType type, void *msg, uint8_t len);


#endif  // __COMMQUEUE_H__
