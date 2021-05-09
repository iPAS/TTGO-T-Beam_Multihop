#include "bind.h"


/**
 * zTimer
 */
void zTimerCreate(zTimer *timer)
{
    timer->timerHandle = NULL;
    timer->callback_fn = NULL;
}

static void vTimerCallback(TimerHandle_t xTimer)
{
    zTimer *timer = pvTimerGetTimerID(xTimer);  // timer ID, but used as argument -- passing zTimer
    TimerFired fn = timer->callback_fn;
    (*fn)(timer);
}

void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, TimerFired timerFired)
{
    timer->callback_fn = timerFired;
    timer->timerHandle = xTimerCreate(
        "Name4DbgOnly",  // The RTOS kernel itself only ever references a timer by its handle, and never by its name.
        pdMS_TO_TICKS(interval),  // Period/time
        (type == TIMER_PERIODIC)? pdTRUE : pdFALSE,  // Auto reload
        timer,  // timer ID, but used as argument -- passing zTimer
        vTimerCallback); /* callback */

    if(xTimerStart(timer->timerHandle, 0) != pdPASS)  // Start it suddenly.
    {
        // The timer could not be set into the Active state.
        // TODO: show something
    }
}

void zTimerStop(zTimer *timer)
{
    xTimerStop(timer->timerHandle, 0);  // Stop it suddenly.
    xTimerReset(timer->timerHandle, 0);  // Reset it
}

uint16_t zTimerTicks()
{
    return millis();
}


/**
 * Radio
 */
Address getAddress()
{
    return 0;  // TODO: how?
}

void radioSetRxHandler(RadioRxHandler rxHandler)
{

}

RadioStatus radioRequestTx(Address dst, MessageType type, const void *msg, uint8_t len, RadioTxDone txDone)
{

}


/**
 * Debug
 */
void debug(char *fmt, ...)
{

}
