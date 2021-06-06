#include "neighbor.h"


static neighbor_t neighbors[MAX_NEIGHBOR];


/**
 * Neighbor information management
 */
void neighbor_init()
{
    uint8_t i;
    for (i = 0; i < MAX_NEIGHBOR; i++)
    {
        neighbors[i].addr = BROADCAST_ADDR;
        neighbors[i].timestamp = 0;
        neighbors[i].rssi = 0;
    }
}


neighbor_t *neighbor_find(Address addr)
{
    neighbor_t *nb = neighbors,
               *nb_free = NULL,
               *nb_oldest = NULL;
    uint32_t ts_oldest = 0,
             now = zTimerTicks();
    uint8_t i;
    for (i = 0; i < MAX_NEIGHBOR; i++, nb++)
    {
        if (nb->addr == BROADCAST_ADDR)
        {
            nb_free = nb;  // Memorize a free slot.
        }
        else
        {
            // Have found!
            if (nb->addr == addr)
            {
                nb->timestamp = now;
                return nb;
            }

            // Not found, find the oldest
            if (nb->timestamp > now)
                nb->timestamp = 0;  // Reset if overflow

            if (nb->timestamp >= ts_oldest)
            {
                ts_oldest = nb->timestamp;
                nb_oldest = nb;
            }
        }
    }

    // In case of not found, select an available slot, or, the oldest.
    nb = (nb_free != NULL)? nb_free : nb_oldest;
    nb->addr = addr;
    nb->timestamp = now;
    return nb;
}


neighbor_t *neighbor_table()
{
    return neighbors;
}
