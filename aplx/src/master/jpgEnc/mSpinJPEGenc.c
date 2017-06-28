#include "mSpinJPEGenc.h"

/*--------------------------- MAIN FUNCTION -----------------------------*/
/*-----------------------------------------------------------------------*/
void c_main()
{
    app_init();

    //spin1_callback_on (MCPL_PACKET_RECEIVED, count_packets, -1);
	//spin1_callback_on (DMA_TRANSFER_DONE, hDMA, 0);
    spin1_callback_on(FRPL_PACKET_RECEIVED, hFR, -1);

	// then go in the event-loop
    spin1_start (SYNC_NOWAIT);
}
/*-----------------------------------------------------------------------*/

/* encode() is the main event loop which is called when a new image arrives
 * */
void encode(uint arg0, uint arg1)
{
	e = jpec_enc_new(sdramImgBuf, wImg, hImg, jpgQ);
	jpgResult = jpec_enc_run(e, &szjpgResult);
	// TODO: send the result to host-PC
	sendJPG((uint)jpgResult, uint(szjpgResult));
	jpec_enc_del(e);
}










/*------------------------ RASTER MAIN FUNCTION ---------------------------*/
/*-------------------------------------------------------------------------*/
void sendJPG(uint imgAddr, uint imgSize)
{
	uint dmaLen, sdpLen;
	uint rem = imgSize, sdpRem;	// remaining data to send
	// first, allocate dtcmImgBuf and its ptr
	dtcmImgBuf = sark_alloc(1, dmaLen);
	uchar *ptr;
	// initialize the dma request size
	dmaLen = 6800; // 6800 == (256+16)*25 == 272*25; --> for 25 stream of sdp
	do {
		dmaLen = dmaLen < rem ? dmaLen : rem;
		// fetch data from sdram
		dmaImgFromSDRAMdone = false;
		spin1_dma_transfer(DMA_FETCH_IMG_FOR_SDP_TAG, (void *)imgAddr,
						   (void *)dtcmImgBuf, DMA_READ, dmaLen);
		while(!dmaImgFromSDRAMdone) {
		}
		// then split into chunks and send the sdp
		ptr = dtcmImgBuf;
		// initialize sdp size
		sdpLen = 272;
		sdpRem = dmaLen;
		do {
			sdpLen = sdpLen < dmaLen ? sdpLen : dmaLen;
			sark_mem_cpy((void *)&sdpResult.cmd_rc, (void *)ptr, sdpLen);
			sdpResult.length = sizeof(sdp_hdr_t) + sdpLen;
			spin1_send_sdp_msg(&sdpResult, 10);
			ptr += sdpLen;
			sdpRem -= sdpLen;
			spin1_delay_us(SDP_TX_TIMEOUT); // otherwise, many sdp packet get dropped
		} while( sdpRem > 0);
		rem -= dmaLen;
	} while(rem > 0);
	// then release dtcmImgBuf
	sark_free(dtcmImgBuf);
	// finally, inform host that transmission is complete
	sdpResult.length = sizeof(sdp_hdr);	// empty message == EOF
	spin1_send_sdp_msg(&sdpResult, 10);
}











/*--------------------------- HELPER FUNCTION -----------------------------*/
/*-------------------------------------------------------------------------*/
/*--- private function ---*/
static void initIPTag()
{
	// make sure ip tag SDP_SEND_RESULT_TAG is set correctly
	sdp_msg_t iptag;
	iptag.flags = 0x07;	// no replay
	iptag.tag = 0;		// internal
	iptag.srce_addr = sv->p2p_addr;
	iptag.srce_port = 0xE0 + coreID;	// use port-7
	iptag.dest_addr = sv->p2p_addr;
	iptag.dest_port = 0;				// send to "root"
	iptag.cmd_rc = 26;
	// set the reply tag
	iptag.arg1 = (1 << 16) + SDP_SEND_RESULT_TAG;
	iptag.arg2 = SDP_SEND_RESULT_PORT;
	iptag.arg3 = SDP_HOST_IP;
	iptag.length = sizeof(sdp_hdr_t) + sizeof(cmd_hdr_t);
	spin1_send_sdp_msg(&iptag, 10);
}

void app_init ()
{
    coreID = spin1_get_core_id();
	// prepare sdp for data
	sdpResult.flags = 7;	// no reply
	sdpResult.tag = SDP_SEND_RESULT_TAG;
	sdpResult.srce_addr = sv->p2p_addr;
	sdpResult.srce_port = (SDP_PORT_RAW_DATA << 5) | coreID;
	sdpResult.dest_addr = sv->dbg_addr;
	sdpResult.dest_port = PORT_ETH;
	// prepare IP Tag
	initIPTag();
}
