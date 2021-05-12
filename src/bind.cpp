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
    zTimerFired fn = timer->callback_fn;
    (*fn)(timer);
}

void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, zTimerFired onFired)
{
    if (timer->timerHandle != NULL)
        zTimerStop(timer);

    timer->callback_fn = onFired;
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
static void zTimerTestFired(zTimer *arg)
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

    debug("Address: %04X", getAddress());
}


/**
 * Radio
 */
Address getAddress()  // TODO: configurable
{
    uint8_t mac[6] = {0};
    esp_efuse_mac_get_default(mac);
    // debug("MAC: %02X %02X %02X %02X %02X %02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return ((Address)mac[4]<<8) | (Address)mac[5];
}


static RadioRxHandler radioRxHandler;

static void loraOnReceive(int packetLength)
{
    uint8_t *msg = (uint8_t *)malloc(packetLength);
    uint8_t *p = msg;
    MessageHeader *hdr = (MessageHeader *)msg;

    while (LoRa.available() > 0)
    {
        *p++ = LoRa.read();
    }

    if (hdr->dst == getAddress()  ||  hdr->dst == BROADCAST_ADDR)
        (*radioRxHandler)(hdr->src, hdr->type, &msg[sizeof(MessageHeader)], hdr->data_len);

    free(msg);
}

void radioSetRxHandler(RadioRxHandler rxHandler)
{
    radioRxHandler = rxHandler;
    LoRa.onReceive(loraOnReceive);
}


static RadioTxDone radioTxDone;

static void loraOnTxDone()
{
    (*radioTxDone)(RADIO_OK);
}

RadioStatus radioRequestTx(Address dst, MessageType type, const void *msg, uint8_t len, RadioTxDone txDone)
{
    if (txDone != NULL)
    {
        radioTxDone = txDone;
        LoRa.onTxDone(loraOnTxDone);
    }
    else
    {
        radioTxDone = NULL;
        LoRa.onTxDone(NULL);
    }

    MessageHeader hdr;
    hdr.src = getAddress();
    hdr.dst = dst;
    hdr.type = type;
    hdr.data_len = len;

    LoRa.beginPacket();
    LoRa.write((uint8_t *)&hdr, sizeof(hdr));
    LoRa.write((uint8_t *)msg, len);
    return (LoRa.endPacket())? RADIO_OK : RADIO_FAILED;
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
