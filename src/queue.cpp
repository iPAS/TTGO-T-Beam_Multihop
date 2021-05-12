#include "queue.h"


void q_init(queue_t *q)
{
    q->head = NULL;
    q->tail = NULL;
    q->len = 0;
}


void q_enqueue(queue_t *q, void *data, uint8_t len)
{
    // For the new item.
    linklist_t *ll = (linklist_t *)malloc(sizeof(linklist_t));
    ll->data = malloc(len);
    memcpy(ll->data, data, len);
    ll->len = len;
    ll->next = NULL;

    // Enqueue
    if (q->len == 0)
    {
        q->head = ll;
        q->tail = ll;
    }
    else
    {
        q->head->next = ll;
        q->head = ll;
    }
    q->len++;
}


uint8_t q_dequeue(queue_t *q, void *data, uint8_t maxlen)
{
    linklist_t *ll = q->tail;
    uint8_t len;

    // Dequeue
    if (q->len == 0)
    {
        return 0;
    }
    q->len--;
    q->tail = q->tail->next;

    // Transfer data and remove allocated memories.
    len = (ll->len < maxlen)? ll->len : maxlen;
    memcpy(data, ll->data, len);
    free(ll->data);
    free(ll);

    return len;
}


uint8_t q_length(queue_t *q)
{
    return q->len;
}


linklist_t *q_item(queue_t *q, uint8_t index)
{
    if (index >= q_length(q))  // No item or out-of-scope
        return NULL;

    linklist_t *spider = q->tail;
    uint8_t i;
    for (i = 0; i < index; i++)
    {
        spider = spider->next;
    }
    return spider;
}
