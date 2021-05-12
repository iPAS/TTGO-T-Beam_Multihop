#include "commqueue.h"


static queue_t cq;
static zTimer delayTxTimer;


/**
 * Sending task
 */
void cq_tx_task(zTimer *timer)
{
    TxTask task;
    q_dequeue(&cq, &task, sizeof(task));

    radioRequestTx(task.dst, task.type, task.msg, task.len, NULL);
    free(task.msg);

    if (q_length(&cq) == 0)  // No more task left
        return;

    TxTask *next = (TxTask *)q_item(&cq, 0)->data;
    zTimerStart(&delayTxTimer, TIMER_ONESHOT, task.delay_tick, &cq_tx_task);
}


/**
 * Send with random delay
 */
bool cq_send(Address dst, MessageType type, void *msg, uint8_t len)
{
    // Prepare a new task.
    TxTask task;
    task.dst = dst;
    task.type = type;
    task.msg = malloc(len);
    memcpy(task.msg, msg, len);
    task.len = len;
    task.delay_tick = rand() % DELAY_TX_GAP;

    // Queue the task
    q_enqueue(&cq, &task, sizeof(task));

    // Start this job if it's the only first one.
    if (q_length(&cq) == 1)
    {
        zTimerStart(&delayTxTimer, TIMER_ONESHOT, task.delay_tick, &cq_tx_task);
    }

    return true;
}


/**
 * Init
 */
void cq_init()
{
    q_init(&cq);
    zTimerCreate(&delayTxTimer);
}
