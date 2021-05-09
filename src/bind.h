#ifndef __BIND_H__
#define __BIND_H__


#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include "all_headers.h"

#define BROADCAST_ADDR 0xFFFF
typedef uint16_t Address;
typedef uint8_t MessageType;

typedef struct
{
    xTimerHandle timerHandle;
} zTimer;

typedef enum
{
    TIMER_ONESHOT,
    TIMER_PERIODIC
} TimerType;

typedef void (*TimerFired)(zTimer*);

extern void zTimerCreate(zTimer *timer);
extern void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, TimerFired timerFired);
extern void zTimerStop(zTimer *timer);
extern uint16_t zTimerTicks();


#endif  // __BIND_H__
