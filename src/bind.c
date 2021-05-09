#include "bind.h"


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
