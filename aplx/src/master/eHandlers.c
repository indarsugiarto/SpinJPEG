#include "mSpinJPEG.h"

/************************************************************************************/
/*------------------------------- SDP EVENT ----------------------------------------*/
/* hSDP - handler for SDP events
 * port-1: host-PC sends jpg data
 * port-2: host-PC sends command, config, etc.
 */
void hSDP (uint mBox, uint port)
{

    sdp_msg_t *msg = (sdp_msg_t *) mBox;
#if(DEBUG_MODE>0)
    //io_printf(IO_STD, "Got msg port-%d with length=%d\n", port, msg->length);
#endif

    if (port == SDP_PORT_JPEG_CMD) {
        if (msg->cmd_rc == SDP_CMD_INIT_SIZE) {
            //resizeImgBuf(msg->arg1, NULL);
            szImgFile = msg->arg1;
            nReceivedChunk = 0;
            spin1_schedule_callback(resizeImgBuf, szImgFile, 0, 1);
#if(DEBUG_MODE>0)
            io_printf(IO_STD, "Sdram buffer is allocated at 0x%x with ptr 0x%x\n", sdramImgBuf, sdramImgBufPtr);
#endif
        }
    }
    else if (port == SDP_PORT_JPEG_DATA) {
        // assuming msg->length contains correct value ???
        uint len = msg->length - sizeof(sdp_hdr_t);
        // Note: GUI will send "header only" to signify end of image data
        if(len > 0) {
            uint tid = spin1_dma_transfer(DMA_IMG_BUF_WRITE, sdramImgBufPtr, 
                               (void *)&msg->cmd_rc, DMA_WRITE, len);

#if(DEBUG_MODE>=0)
            if(tid == 0) dmaAllocErrCntr++;
#endif

            sdramImgBufPtr += len;
            nReceivedChunk++;
#if(DEBUG_MODE>0)
            //io_printf(IO_STD, "dma with tid-%d is requested\n", tid);
            io_printf(IO_BUF, "sdramImgBufPtr is at 0x%x\n", sdramImgBufPtr);
#endif
        } 
        // end of image data detected
        else {
            io_printf(IO_STD, "[INFO] End-of-data is detected!\n");
            io_printf(IO_STD, "[INFO] %d-chunks for %d-byte data are collected!\n",nReceivedChunk,szImgFile);
            io_printf(IO_STD, "[INFO] dmaAllocErrCntr = %d\n", dmaAllocErrCntr);
#if(DEBUG_MODE>0)
            io_printf(IO_STD, "[INFO] Received %d chunks\n", nReceivedChunk);
#endif
        }
    }

    spin1_msg_free (msg);
}



/************************************************************************************/
/*------------------------------- DMA DONE EVENT -----------------------------------*/
void hDMA (uint tid, uint tag)
{
#if(DEBUG_MODE>0)
            //io_printf(IO_STD, "[INFO] DMA with tid-%d is done\n", tid);
#endif
    /* when the first chunk of jpeg file is received, trigger the main decoder */
    if(tag == DMA_IMG_BUF_WRITE && nReceivedChunk == 0) {
        spin1_trigger_user_event(UE_START_DECODER, NULL);
    }
}



/************************************************************************************/
/*------------------------------- USER EVENT ---------------------------------------*/
void hUEvent(uint eventID, uint arg)
{
    switch(eventID) {
    case UE_START_DECODER:
        spin1_schedule_callback(decode, NULL, NULL, 1);
        break;
    }
}
