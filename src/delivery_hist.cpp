#include "delivery_hist.h"


static delivery_history_t delivery_history[MAX_HISTORY];


/**
 * Delivery history management
 */
void hist_init()
{
    uint8_t i;
    for (i = 0; i < MAX_HISTORY; i++)
    {
        delivery_history[i].available = false;
        delivery_history[i].timestamp = 0;
        delivery_history[i].parent = BROADCAST_ADDR;
    }
}


delivery_history_t *hist_find(RoutingHeader *hdr)
{
    delivery_history_t *hist = delivery_history,
                       *hist_free = NULL,
                       *hist_oldest = NULL;
    uint32_t ts_oldest = 0,
             now = zTimerTicks();
    uint8_t i;

    // Find in the table
    for (i = 0; i < MAX_HISTORY; i++, hist++)
    {
        if (hist->available == false)
        {
            hist_free = hist;  // Memorize a free slot.
        }
        else
        {
            // Have found!
            if (hist->latestHdr.originSource == hdr->originSource)
            {
                hist->timestamp = now;
                return hist;
            }

            // Not found, find the oldest
            if (hist->timestamp > now)  // Long-time no see, forget it.
            {
                hist->available = true;
                hist->timestamp = 0;
                hist->parent = BROADCAST_ADDR;
                hist_free = hist;  // Memorize a free slot.
                continue;
            }

            if (hist->timestamp >= ts_oldest)
            {
                ts_oldest = hist->timestamp;
                hist_oldest = hist;
            }
        }
    }

    // In case of not found, select an available slot, or, the oldest.
    hist = (hist_free != NULL)? hist_free : hist_oldest;
    hist->available = true;
    hist->timestamp = now;
    memcpy(&hist->latestHdr, hdr, sizeof(hist->latestHdr));
    hist->currSeqNo = hdr->seqNo - 1;  // Believe in the sender
    hist->parent = BROADCAST_ADDR;  // Back to none again.
    return hist;
}
