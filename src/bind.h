#ifndef __BIND_H__
#define __BIND_H__


#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include "all_headers.h"

#define SIZE_DEBUG_BUF 255

/**
 * Timer
 */
typedef struct zTimer
{
    xTimerHandle timerHandle;
    void (*callback_fn)(struct zTimer *timer);
} zTimer;

typedef void (*zTimerFired)(zTimer *timer);

typedef enum
{
    TIMER_ONESHOT,
    TIMER_PERIODIC
} TimerType;

extern void zTimerCreate(zTimer *timer);
extern void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, zTimerFired onFired);
extern void zTimerStop(zTimer *timer);
extern uint16_t zTimerTicks();
extern void zTimerTest();


/**
 * Radio
 */
#define BROADCAST_ADDR ((Address)0xFFFF)
typedef uint16_t Address;
typedef uint8_t MessageType;

typedef struct __attribute__((packed))
{
    // uint8_t frame_id;  // No need. Of motelib only
    Address src;
    Address dst;
    uint8_t data_len;

    MessageType type;
    // uint8_t seq_no;  // No need. Of motelib only
} MessageHeader;

typedef enum
{
    RADIO_OK,     ///< Previous request is ready or successful
    RADIO_FAILED, ///< Previous request is not ready or not successful
} RadioStatus;

typedef void (*RadioRxHandler)(Address source, MessageType type, void *message, uint8_t len);
typedef void (*RadioTxDone)(RadioStatus status);

extern Address getAddress();
extern void radioSetRxHandler(RadioRxHandler rxHandler);
extern RadioStatus radioRequestTx(Address dst, MessageType type, const void *msg, uint8_t len, RadioTxDone txDone);


/**
 * Others
 */
extern void debug(const char *format, ...);


#endif  // __BIND_H__
