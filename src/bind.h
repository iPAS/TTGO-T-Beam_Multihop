#ifndef __BIND_H__
#define __BIND_H__


#include <freertos/FreeRTOS.h>

#include <BluetoothSerial.h>
#include <SimpleCLI.h>

#include "all_headers.h"


#define SIZE_DEBUG_BUF 255

#define term_print(arg)   { Serial.print(arg);   if (bt.connected()) bt.print(arg); }
#define term_println(arg) { Serial.println(arg); if (bt.connected()) bt.println(arg); }

#define debug(args...) term_printf("[X] " args)
extern void term_printf(const char *format, ...);

typedef enum {
    R_NODE_ID,
} pref_reg_t;


/**
 * Timer
 */
typedef struct zTimer
{
    xTimerHandle timerHandle;
    TickType_t period_tick;
    UBaseType_t do_reload;
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
#define LORA_CALLBACK_MODE

#define LORARECV_TASK_STACK_SIZE 1024
#define LORARECV_TASK_CORE_ID 0

#define SINK_ADDRESS ((Address)0)
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
extern Address setAddress(Address addr);
extern void radioSetRxHandler(RadioRxHandler rxHandler);
extern RadioStatus radioRequestTx(Address dst, MessageType type, const void *msg, uint8_t len, RadioTxDone txDone);
extern void loraOnReceive(int packetLength);
extern void radio_setup();


/**
 * Testers
 */
extern void test_ztimer();


/**
 * Global, seen by default but prevent ide confused
 */
extern BluetoothSerial bt;
extern SimpleCLI cli;
extern void oled_update_display();
extern void vtube_command_to_station(String cmd);
extern void lora_receive();

extern void config_save(pref_reg_t reg);
extern void config_load(pref_reg_t reg);


#endif  // __BIND_H__
