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
                       *free_hist = NULL,
                       *oldest_hist = NULL;
    uint32_t oldest_timestamp = 0,
             now = timerTicks();
    uint8_t i;

    // Find in the table
    for (i = 0; i < MAX_HISTORY; i++, hist++)
    {
        if (!hist->available)
        {
            free_hist = hist;  // Memorize a free slot.
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
            if (hist->timestamp > now)
                hist->timestamp = 0;  // Reset if overflow

            if (hist->timestamp >= oldest_timestamp)
            {
                oldest_timestamp = hist->timestamp;
                oldest_hist = hist;
            }
        }
    }

    // In case of not found, select an available slot, or, the oldest.
    hist = (free_hist != NULL)? free_hist : oldest_hist;
    hist->available = true;
    hist->timestamp = now;
    memcpy(&hist->latestHdr, hdr, sizeof(hist->latestHdr));
    hist->currSeqNo = hdr->seqNo - 1;  // Believe in the sender
    hist->parent = BROADCAST_ADDR;  // Back to none again.
    return hist;
}
