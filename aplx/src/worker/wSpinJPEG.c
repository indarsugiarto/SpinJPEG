/* spinjpeg.c is the main part that works as a master
 * In this file, the main decoder loop is implemented.
 * It will broadcast messages to workers when an intense computation is
 * found.
 *
 * */
#include "spinjpeg.h"

/*--------------------------- MAIN FUNCTION -----------------------------*/
/*-----------------------------------------------------------------------*/
void c_main()
{
    app_init();

  //spin1_callback_on (MCPL_PACKET_RECEIVED, count_packets, -1);
    spin1_callback_on (DMA_TRANSFER_DONE, hDMA, 0);

	// then go in the event-loop
    spin1_start (SYNC_NOWAIT);
}
/*-----------------------------------------------------------------------*/


/*--------------------------- HELPER FUNCTION -----------------------------*/
/*-------------------------------------------------------------------------*/

void app_init ()
{
    coreID = spin1_get_core_id ();
    if(coreID == SDP_RECV_CORE_ID) {
        spin1_callback_on (SDP_PACKET_RX, hSDP, -1);
    }
    sdramImgBufInitialized = false;
}

/* Round up to the next highest power of 2 
 * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
uint roundUp(uint v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/* resizeImgBuf() is called when host-PC trigger an SDP with SDP_CMD_INIT_SIZE
 *
 */
void resizeImgBuf(uint szFile, uint null)
{
    bool createNew;
    if(sdramImgBufInitialized) {
        if(szFile > sdramImgBufSize) {
           sark_xfree(sv->sdram_heap, sdramImgBuf, ALLOC_LOCK);
           createNew = true;
        } else {
           // nothing to change, just
           // reset the image buffer pointer to the starting position
           sdramImgBufPtr = sdramImgBuf;
           createNew = false;
#if(DEBUG_MODE > 0)
           io_printf(IO_STD, "No need to create a new sdram buffer\n");
           io_printf(IO_STD, "sdramImgBufPtr is reset to 0x%x\n", sdramImgBufPtr);
#endif
        }
    } else {
        createNew = true;
    }

    if(createNew) {
        /* From sark.h regarding sark_xalloc:
           Returns NULL on failure (not enough memory, bad tag or tag in use).

           The flag parameter contains two flags in the bottom two bits. If bit 0
           is set (ALLOC_LOCK), the heap manipulation is done behind a lock with
           interrupts disabled. If bit 1 is set (ALLOC_ID), the block is tagged
           with the AppID provided in bits 15:8 of the flag, otherwise the AppID
           of the current application is used.

           The 8-bit "tag" is combined with the AppID and stored in the "free"
           field while the block is allocated. If the "tag" is non-zero, the
           block is only allocated if the tag is not in use.  The tag (and AppID)
           is stored in the "alloc_tag" table while the block is allocated.

           Hence, flag is set only to ALLOC_LOC: behind lock
                  tag is set to 0, to make sure it is always allocated (using AppID) 
        */
        sdramImgBufSize = roundUp(szFile);

#if(DEBUG_MODE > 0)
        io_printf(IO_STD, "Allocating %d-bytes of sdram buffer\n", sdramImgBufSize);
#endif

        sdramImgBuf = sark_xalloc (sv->sdram_heap, szFile, 0, ALLOC_LOCK);
        if(sdramImgBuf != NULL) {
            sdramImgBufInitialized = true;
            sdramImgBufPtr = sdramImgBuf;
#if(DEBUG_MODE > 0)
            io_printf(IO_BUF, "Sdram buffer is allocated at 0x%x with ptr 0x%x\n", sdramImgBuf, sdramImgBufPtr);
#endif
        } else {
            io_printf(IO_BUF, "[ERR] Fail to create sdramImgBuf!\n");
            // dangerous: terminate then!
            rt_error (RTE_MALLOC);
            sdramImgBufInitialized = false;
        }
    }
}

