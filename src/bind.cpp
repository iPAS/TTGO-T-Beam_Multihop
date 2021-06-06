#include "bind.h"

#include <SPI.h>
#include <LoRa.h>


/******************************************************************************
 * zTimer
 */
static void zTimerInit(zTimer *timer)
{
    timer->timerHandle = NULL;
    timer->callback_fn = NULL;
}


void zTimerCreate(zTimer *timer)
{
    zTimerInit(timer);
}


static void vTimerCallback(TimerHandle_t xTimer)
{
    zTimer *timer = (zTimer *)pvTimerGetTimerID(xTimer);  // timer ID, but used as argument -- passing zTimer
    if (timer->callback_fn == NULL)
        debug("vTimerCallback() timer->callback_fn == NULL");
    else
        (*timer->callback_fn)(timer);
}


void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, zTimerFired onFired)
{
    TickType_t period_tick = pdMS_TO_TICKS(interval);
    UBaseType_t do_reload = (type == TIMER_PERIODIC)? pdTRUE : pdFALSE;

    timer->callback_fn = onFired;

    if (timer->timerHandle != NULL)  // Already created and started.
    {
        if (xTimerIsTimerActive(timer->timerHandle))
            xTimerStop(timer->timerHandle, 0);

        if (timer->period_tick != period_tick)
        {
            timer->period_tick = period_tick;
            if (xTimerChangePeriod(timer->timerHandle, period_tick, 0) == pdFAIL)
                debug("xTimerChangePeriod() problem");
        }

        if (timer->do_reload != do_reload)
        {
            if (xTimerDelete(timer->timerHandle, 0) == pdFAIL)
                debug("xTimerDelete() problem");
            timer->timerHandle = NULL;
        }
    }

    if (timer->timerHandle == NULL)
    {
        timer->period_tick = period_tick;
        timer->do_reload = do_reload;

        timer->timerHandle = xTimerCreate(
            "Name4DbgOnly",         // Never be referred by the kernel.
            period_tick,            // Period/time
            do_reload,              // Auto reload
            timer,                  // Adapted for passing argument.
            vTimerCallback);        // The callback

        if (timer->timerHandle == NULL)
        {
            debug("xTimerCreate() problem");
            return;
        }
    }

    if(xTimerStart(timer->timerHandle, 0) == pdFAIL)  // Start it suddenly.
    {
        debug("xTimerStart() problem");
    }
}


void zTimerStop(zTimer *timer)
{
    xTimerStop(timer->timerHandle, 0);  // Stop it suddenly.
    // xTimerReset(timer->timerHandle, 0);  // Reset & if not already been started, will start the timer.
    // xTimerDelete(timer->timerHandle, 0);  // Delete.
    // zTimerInit(timer);  // Re-init
}


uint16_t zTimerTicks()
{
    return millis();
}


static void zTimerTestFired(zTimer *arg)
{
    static uint8_t counter = 0;
    zTimer *timer = (zTimer *)arg;

    if (++counter > 15)
    {
        zTimerStop(timer);
        debug("zTimer Test stop for a while");
        counter = 0;
    }
    else
    {
        uint16_t period = 1000 + (rand() % 1000);
        zTimerStart(timer, TIMER_ONESHOT, period, zTimerTestFired);
        debug("zTimer Test #%d, next in %d ms", counter, period);
    }
}


void test_ztimer()
{
    static uint32_t next = 0;

    while (true)
    {
        if (millis() > next)
        {
            debug("Address: %04X", getAddress());

            static zTimer timer;
            zTimerCreate(&timer);
            zTimerStart(&timer, TIMER_ONESHOT, 1000, zTimerTestFired);

            next = millis() + 30000;
        }
    }
}


/******************************************************************************
 * Radio
 */
static Address node_address = BROADCAST_ADDR;

static RadioRxHandler radioRxHandler;
static QueueHandle_t handleLoRaRecvQueue;
static TaskHandle_t handleLoRaRecvTask;
static RadioTxDone radioTxDoneHandler;


Address getAddress()
{
    Address addr = node_address;
    if (addr == BROADCAST_ADDR)
    {
        uint8_t mac[6] = {0};
        esp_efuse_mac_get_default(mac);
        // debug("MAC: %02X %02X %02X %02X %02X %02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        addr = ((Address)mac[4]<<8) | (Address)mac[5];
    }
    return addr;
}


Address setAddress(Address addr)
{
    node_address = addr;
    return node_address;
}


void radioGetRxStatus(RadioRxStatus* status)
{
    // TODO:
}


static void loraOnReceiveTask(void *pvParameters)
{
    uint8_t msg[512];

    while (true)
    {
        // vTaskSuspend(NULL);

        LoRaRecvQueueItem_t item;
        if (xQueueReceive(handleLoRaRecvQueue, &item, portMAX_DELAY) != pdTRUE)
            continue;

        uint16_t packetLength = item.packet_length;
        uint8_t *msg = (uint8_t *)malloc(packetLength);
        if (msg == NULL)
        {
            debug("loraOnReceive() cannot allocate memory");
            return;
        }

        uint8_t *p = msg;
        MessageHeader *hdr = (MessageHeader *)msg;
        uint16_t i;
        for (i = 0; i < packetLength  &&  LoRa.available() > 0; i++)
        {
            *p++ = LoRa.read();
        }

        if (hdr->dst == getAddress()  ||  hdr->dst == BROADCAST_ADDR)
        {
            (*radioRxHandler)(hdr->src, hdr->type, &msg[sizeof(MessageHeader)], hdr->data_len);
        }
        free(msg);
    }

    vTaskDelete(NULL);
}


void loraOnReceive(int packetLength)
{
    // vTaskResume(handleLoRaRecvTask);

    BaseType_t xHigherPrioritTaskWoken = pdTRUE;  // The task was woken at the start of the ISR.
    LoRaRecvQueueItem_t item;
    item.packet_length = packetLength;
    xQueueSendFromISR(handleLoRaRecvQueue, &item, &xHigherPrioritTaskWoken);
    if(xHigherPrioritTaskWoken)
    {
        // taskYIELD();
        portYIELD_FROM_ISR();
    }
    return;


    uint8_t *msg = (uint8_t *)malloc(packetLength);
    if (msg == NULL)
    {
        debug("loraOnReceive() cannot allocate memory");
        return;
    }

    uint8_t *p = msg;
    MessageHeader *hdr = (MessageHeader *)msg;
    uint16_t i;
    for (i = 0; i < packetLength  &&  LoRa.available() > 0; i++)
    {
        *p++ = LoRa.read();
    }

    if (hdr->dst == getAddress()  ||  hdr->dst == BROADCAST_ADDR)
    {
        (*radioRxHandler)(hdr->src, hdr->type, &msg[sizeof(MessageHeader)], hdr->data_len);
    }
    free(msg);
}


void radioSetRxHandler(RadioRxHandler rxHandler)
{
    radioRxHandler = rxHandler;

    LoRa.onReceive(loraOnReceive);  // XXX: Change from interrupt routine to a function call inside loop().
                                    // This function will be called by lora_parsing_process() from lora.ino
    LoRa.receive();  // Begin reception-mode
}


RadioStatus radioRequestTx(Address dst, MessageType type, const void *msg, uint8_t len, RadioTxDone txDone)
{
    radioTxDoneHandler = (txDone == NULL)? NULL : txDone;

    MessageHeader hdr;
    hdr.src = getAddress();
    hdr.dst = dst;
    hdr.type = type;
    hdr.data_len = len;

    LoRa.beginPacket();
    LoRa.write((uint8_t *)&hdr, sizeof(hdr));
    LoRa.write((uint8_t *)msg, len);
    RadioStatus ret = (LoRa.endPacket())? RADIO_OK : RADIO_FAILED;

    LoRa.receive();  // Back to reception-mode

    if (radioTxDoneHandler != NULL)
        (*radioTxDoneHandler)(ret);

    return ret;
}


void radio_setup()
{
    handleLoRaRecvQueue = xQueueCreate(LORARECV_Q_SIZE, LORARECV_Q_ITEM_SIZE);
    if (handleLoRaRecvQueue == NULL)
    {
        debug("xQueueCreate() problem");
        while (1);
    }

    if (xTaskCreate(
            loraOnReceiveTask,      // Routine
            "LoRaRecvTask",         // Task's name
            LORARECV_TASK_STACK_SIZE,  // Stack size
            NULL,                   // pvParameters
            uxTaskPriorityGet(NULL)+1,  // Priority -- https://blog.bachi.net/?p=9690
            &handleLoRaRecvTask    // Task's data structure
            ) != pdPASS)
    {
        debug("xTaskCreate() problem");
        while (1);
    }
}


/******************************************************************************
 * For debugging
 */
void term_printf(const char *format, ...)
{
    static char buf[SIZE_DEBUG_BUF];
    char *p = buf;
    va_list ap;
    va_start(ap, format);
    vsnprintf(p, sizeof(buf), format, ap);

    term_println(buf);

    va_end(ap);
}
