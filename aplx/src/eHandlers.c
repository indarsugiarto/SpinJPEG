#include "spinjpeg.h"

/* hSDP - handler for SDP events
 * port-1: host-PC sends jpg data
 * port-2: host-PC sends command, config, etc.
 */
void hSDP (uint mBox, uint port)
{
    sdp_msg_t *msg = (sdp_msg_t *) mBox;

    if (port == SDP_PORT_JPEG_CMD) {
        if (msg->cmd_rc == SDP_CMD_INIT_SIZE) {
            resizeImgBuf(msg->arg1, NULL);
        }
    }
    else if (port == SDP_PORT_JPEG_DATA) {
        // assuming msg->length contains correct value ???
        uint len = msg->length - sizeof(sdp_hdr_t);
        // Note: GUI will send "header only" to signify end of image data
        if(len > 0) {
            spin1_dma_transfer(DMA_IMG_BUF_WRITE, sdramImgBufPtr, 
                               (void *)&msg->cmd_rc, DMA_WRITE, len);
            sdramImgBufPtr += len;
        } 
        // end of image data detected
        else {
            io_printf(IO_BUF, "[INFO] End-of-data is detected!\n");
        }
    }

    spin1_msg_free (msg);
}

