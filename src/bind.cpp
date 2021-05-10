#include "bind.h"

#include <SPI.h>
#include <LoRa.h>
#include <stdarg.h>


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
    zTimer *timer = (zTimer *)pvTimerGetTimerID(xTimer);  // timer ID, but used as argument -- passing zTimer
    TimerFired fn = timer->callback_fn;
    (*fn)(timer);
}

void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, TimerFired timerFired)
{
    if (timer->timerHandle != NULL)
        zTimerStop(timer);

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
    xTimerReset(timer->timerHandle, 0);  // Reset it without waiting.
    xTimerDelete(timer->timerHandle, 0);  // Delete.
    zTimerCreate(timer);  // Re-init
}

uint16_t zTimerTicks()
{
    return millis();
}


/**
 * zTimer Test
 */
static void zTimerTestFired(void *arg)
{
    static uint8_t counter = 0;
    zTimer *timer = (zTimer *)arg;

    if (++counter > 15)
    {
        zTimerStop(timer);
        counter = 0;
    }
    else
    {
        zTimerStart(timer, TIMER_PERIODIC, 1000, zTimerTestFired);
        debug("zTimer Test #%d", counter);
    }
}

void zTimerTest()
{  // Used for testing
    static zTimer timer;
    zTimerCreate(&timer);
    zTimerStart(&timer, TIMER_PERIODIC, 1000, zTimerTestFired);
}


/**
 * Radio
 */
Address getAddress()
{
    // esp_efuse_mac_get_default()
    return 0;  // TODO: how?
}

static RadioRxHandler radioRxHandler;

static void radioOnReceive(int packetLength)
{
    // TODO: rearrange format

    // (*radioRxHandler)(Address source, MessageType type, void *message, uint8_t len);
}

void radioSetRxHandler(RadioRxHandler rxHandler)
{
    radioRxHandler = rxHandler;
    LoRa.onReceive(radioOnReceive);
}

RadioStatus radioRequestTx(Address dst, MessageType type, const void *msg, uint8_t len, RadioTxDone txDone)
{
    // TODO:

    // void on_tx_done() {
    //     Serial.println(counter);
    // }
    // LoRa.onTxDone(on_tx_done);

    // LoRa.beginPacket();
    // LoRa.printf("<%04X> #", ESP.getEfuseMac());
    // LoRa.print(counter);
    // LoRa.endPacket();
}


/**
 * Debug
 */
void debug(const char *format, ...)
{
    char buf[SIZE_DEBUG_BUF], *p = buf;
    va_list ap;
    va_start(ap, format);
    p += sprintf(p, "[X] ");
    vsnprintf(p, sizeof(buf)-4, format, ap);
    Serial.println(buf);
    va_end(ap);
}
