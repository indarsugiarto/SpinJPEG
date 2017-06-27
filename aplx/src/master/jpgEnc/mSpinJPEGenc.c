#include "mSpinJPEGenc.h"

/*--------------------------- MAIN FUNCTION -----------------------------*/
/*-----------------------------------------------------------------------*/
void c_main()
{
    app_init();

    //spin1_callback_on (MCPL_PACKET_RECEIVED, count_packets, -1);
    spin1_callback_on (DMA_TRANSFER_DONE, hDMA, 0);
    spin1_callback_on(USER_EVENT, hUEvent, 2);

        // then go in the event-loop
    spin1_start (SYNC_NOWAIT);
}
/*-----------------------------------------------------------------------*/












/*------------------------ RASTER MAIN FUNCTION ---------------------------*/
/*-------------------------------------------------------------------------*/












/*--------------------------- HELPER FUNCTION -----------------------------*/
/*-------------------------------------------------------------------------*/

void app_init ()
{
    coreID = spin1_get_core_id();
}
