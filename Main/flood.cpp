#include "flood.h"
#include "delivery_hist.h"
#include "neighbor.h"


static uint8_t txSeqNo;
static on_rx_sink on_approach_sink;  // Handler called if being the last node in the route.


/**
 * Broadcast the message
 */
static
bool broadcast(void *message, uint8_t len)
{
    RoutingHeader *hdr = (RoutingHeader*)message;
    hdr->hopCount++;
    // return cq_send(BROADCAST_ADDR, FLOOD_MSG_TYPE, hdr, sizeof(*hdr));  // XXX: for debugging
    return cq_send(BROADCAST_ADDR, FLOOD_MSG_TYPE, message, len);
}


/**
 * On packet received
 */
static
void on_receive(Address source, MessageType type, void *message, uint8_t len)
{
    neighbor_update_info(source);  // Update date about our neighbor 'source'

    RoutingHeader *hdr = (RoutingHeader*)message;

    delivery_history_t *hist;
    hist = (hdr->originSource == getAddress())? NULL : hist_find(hdr);  // Historical data based on the 'originSource'.
                                                                        // It could be NULL if the 'originSource' is here.

    // ------------------------------------------------------------------------
    if (type == FLOOD_MSG_TYPE)
    {
        // ---------------------------------------------
        // Flood message is back to the origin, discard!
        // ---------------------------------------------
        if (hdr->originSource == getAddress())
        {
           debug("Loopback message seqNo %d from node %d, discard", hdr->seqNo, source);
        }

        // --------------------------------
        // Flood message received correctly
        // --------------------------------
        else
        if (hdr->seqNo > hist->currSeqNo  ||
            (hdr->seqNo == 0 && hist->currSeqNo == 255)  // On overflow
            )
        {
            // Update parent
            debug("Change parent from %d to %d on origin @%d", hist->parent, source, hdr->originSource);
            hist->parent = source;
            debug("New best hop %d", hdr->hopCount);

            // Update with the newest greater seqNo
            debug("Change seqNo from current %d to %d", hist->currSeqNo, hdr->seqNo);
            hist->currSeqNo = hdr->seqNo;

            // If we are the final node!! -- the sink, then
            if (hdr->finalSink == getAddress())
            {
                // -----------------------------------------------
                // The message finally approaches its destination.
                // -----------------------------------------------
                if (on_approach_sink != NULL)
                    on_approach_sink(message, len);
            }
            // If not, rebroadcast
            else
            if (hdr->hopCount < MAX_HOP)
            {
                // -----------
                // Rebroadcast
                // -----------
                broadcast(message, len);
            }

        }

        // -------------------------------------
        // Flood message is duplicated, discard!
        // -------------------------------------
        else
        if (hdr->seqNo == hist->currSeqNo)
        {
           debug("Duplicated seqNo %d from node %d, discard", hdr->seqNo, source);
        }

        // ---------------------------------------------------------------------------
        // Flood message's seqNo lacks to the whole network, report back to the origin
        //  along the route through the source node.
        // Maybe the origin source was disconnected or shut down for a while.
        // ---------------------------------------------------------------------------
        else  // if (hdr->seqNo < hist->currSeqNo)
        {
            // --------------------------------------
            // Report back to the sender -- 'source'.
            // --------------------------------------
            debug("Report seqNo %d < %d to node %d", hdr->seqNo, hist->currSeqNo, source);

            // Tell the upper node -- current sender -- that this node has a greater seqNo.
            // Report it back, up until the first hop.
            RoutingHeader fwd_hdr;
            memcpy(&fwd_hdr, hdr, sizeof(fwd_hdr));
            fwd_hdr.seqNo = hist->currSeqNo;  // Report with a greater 'seqNo' of this node.
            fwd_hdr.hopCount = MAX_HOP - hdr->hopCount;  // Prevent out-of-path routing.
            cq_send(source, REPORT_MSG_TYPE, &fwd_hdr, sizeof(fwd_hdr));
        }
    }
    // ------------------------------------------------------------------------
    else
    if (type == REPORT_MSG_TYPE)
    {
        if (hdr->hopCount < MAX_HOP)
        {
            // --------------------------------------------------------------------
            // The 'originSource' got suggestion with the greater-seqNo correction.
            // --------------------------------------------------------------------
            if (hdr->originSource == getAddress())
            {
                if (hdr->seqNo > txSeqNo  ||
                    (hdr->seqNo == 0 && txSeqNo == 255)  // On overflow
                    )
                {
                    debug("Change seqNo from current %d to %d by suggestion", txSeqNo, hdr->seqNo);
                    txSeqNo = hdr->seqNo;
                }
            }

            // ------------------------------
            // Report message to be forwarded
            // ------------------------------
            else
            if (hdr->seqNo > hist->currSeqNo  ||
                (hdr->seqNo == 0 && hist->currSeqNo == 255)  // On overflow
                )
            {
                debug("Change seqNo from current %d to %d", hist->currSeqNo, hdr->seqNo);
                hist->currSeqNo = hdr->seqNo;  // Update to fix the late currSeqNo at this node

                if (hist->parent != BROADCAST_ADDR)  // Tell the others
                {
                    // -----------------------------------------
                    // Next hop to be reported is node's parent.
                    // -----------------------------------------
                    debug("Report forward seqNo %d from node %d to parent %d", hdr->seqNo, source, hist->parent);

                    RoutingHeader fwd_hdr;
                    memcpy(&fwd_hdr, hdr, sizeof(fwd_hdr));
                    fwd_hdr.seqNo = hist->currSeqNo;
                    fwd_hdr.hopCount++;
                    cq_send(hist->parent, REPORT_MSG_TYPE, &fwd_hdr, sizeof(fwd_hdr));
                }
            }
        }
    }

    // Update the history table with the latest header
    if (hist != NULL)
    {
        memcpy(&hist->latestHdr, hdr, sizeof(hist->latestHdr));
    }
}


/**
 * Set the callback function on reception.
 */
void flood_set_rx_handler(on_rx_sink fn)
{
    on_approach_sink = fn;
}


/**
 * Send to
 */
bool flood_send_to(Address sink, const void *msg, uint8_t len)
{
    debug("Tx to finalSink %d with seqNo %d ", sink, txSeqNo);

    uint8_t length = sizeof(RoutingHeader) + len;
    uint8_t *message = (uint8_t *)malloc(length);
    if (message == NULL)
    {
        debug("flood_send_to() cannot allocate memory");
        return false;
    }

    RoutingHeader *hdr = (RoutingHeader *)message;
    hdr->seqNo = txSeqNo++;
    hdr->hopCount = 0;
    hdr->originSource = getAddress();
    hdr->finalSink = sink;
    memcpy(&message[sizeof(RoutingHeader)], msg, len);

    bool ret = broadcast(message, length);

    free(message);
    return ret;
}


/**
 * Init
 */
void flood_init(void)
{
    cq_init();  // Initial communication queue
    hist_init();  // Initial delivery history for memeorizing a received packet.
    neighbor_init();  // Initial the info. man. of neighbor relation.

    txSeqNo = 1;  // Other nodes will assume that 'originSource' has a greater seqNo than themselves.
    on_approach_sink = NULL;
    radioSetRxHandler(on_receive);
}
