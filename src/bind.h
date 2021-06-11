#ifndef __BIND_H__
#define __BIND_H__


#include <freertos/FreeRTOS.h>

#include <SimpleCLI.h>

#include "all_headers.h"


#define SIZE_DEBUG_BUF 255

#define term_print(arg)   { Serial.print(arg); }
#define term_println(arg) { Serial.println(arg); }
extern void term_printf(const char *format, ...);

#define debug(args...) term_printf("[X] " args)
#ifndef debug
#define debug(args...)
#endif

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

extern void test_ztimer();


/**
 * Radio
 */
#define LORARECV_Q_SIZE 10
#define LORARECV_Q_ITEM_SIZE sizeof(LoRaRecvQueueItem_t)

typedef struct
{
    uint16_t packet_length;
} LoRaRecvQueueItem_t;

#define LORARECV_TASK_STACK_SIZE 8192

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

typedef struct
{
    uint8_t rssi;
    float snr;
} RadioRxStatus;

typedef void (*RadioRxHandler)(Address source, MessageType type, void *message, uint8_t len);
typedef void (*RadioTxDone)(RadioStatus status);

extern Address getAddress();
extern Address setAddress(Address addr);
extern void radioGetRxStatus(RadioRxStatus* status);
extern void radioSetRxHandler(RadioRxHandler rxHandler);
extern RadioStatus radioRequestTx(Address dst, MessageType type, const void *msg, uint8_t len, RadioTxDone txDone);
extern void loraOnReceive(int packetLength);
extern void radio_setup();


/**
 * Neighbor
 */
typedef struct
{
    Address addr;
    uint32_t timestamp;

    uint8_t rssi;
    float snr;
} neighbor_t;

typedef struct __attribute__((packed))  // For sending through the network.
{
    Address addr;
    uint8_t rssi;   // Degree 0-255 of RSSI
    float snr;
} neighbor_status_t;


/**
 * Global, seen by default but prevent ide confused
 */
extern SimpleCLI cli;

extern void oled_update_display();

extern void gps_update_data();
extern char *gps_update_str(const char *fmt);

extern void vtube_command_to_station(String cmd);

extern void lora_receive();
extern bool report_status_to(Address sink);
extern bool report_gps_to(Address sink);

extern void config_save(pref_reg_t reg);
extern void config_load(pref_reg_t reg);


#endif  // __BIND_H__
