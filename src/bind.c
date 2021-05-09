#include "bind.h"


/**
 * zTimer
 */
void zTimerCreate(zTimer *timer)
{
    // timerHndl1Sec = xTimerCreate(
    //   "timer1Sec", /* name */
    //   pdMS_TO_TICKS(1000), /* period/time */
    //   pdTRUE, /* auto reload */
    //   (void*)0, /* timer ID */
    //   vTimerCallback1SecExpired); /* callback */
}

void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, TimerFired timerFired)
{

}

void zTimerStop(zTimer *timer)
{

}

uint16_t zTimerTicks()
{

}


/**
 * Radio
 */
Address getAddress()
{

}

void radioSetRxHandler(RadioRxHandler rxHandler)
{

}

RadioStatus radioRequestTx(Address dst, MessageType type, const void *msg, uint8_t len, RadioTxDone txDone)
{

}


/**
 * 
 */
void debug(char *fmt, ...)
{

}
